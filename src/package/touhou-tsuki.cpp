#include "touhou-tsuki.h"

#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "card.h"

class ThSuoming: public TriggerSkill{
public:
    ThSuoming(): TriggerSkill("thsuoming"){
        events << Damaged << ChainStateChanged << TurnedOver;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        if (triggerEvent == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();
            QStringList skills;
            for (int i = 0; i < damage.damage; i++) {
                skills << objectName();
            }
            return skills;
        } else if (triggerEvent == ChainStateChanged && player->isChained())
            return QStringList(objectName());
        else if (triggerEvent == TurnedOver)
            return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName(), "@thsuoming", true, true);
        if (target) {
            room->broadcastSkillInvoke(objectName());
            player->tag["ThSuomingTarget"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = player->tag["ThSuomingTarget"].value<ServerPlayer *>();
        player->tag.remove("ThSuomingTarget");
        if (target) {
            target->setChained(!target->isChained());
            room->broadcastProperty(target, "chained");
            room->setEmotion(target, "chain");
            room->getThread()->trigger(ChainStateChanged, room, target);
        }
        return false;
    }
};

class ThChiwu: public TriggerSkill {
public:
    ThChiwu(): TriggerSkill("thchiwu") {
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player) || !player->isChained()) return QStringList();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if (effect.card->isKindOf("Duel")
            || (effect.card->isKindOf("Slash") && !effect.card->isKindOf("NatureSlash")))
                return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(player, objectName());

        CardEffectStruct effect = data.value<CardEffectStruct>();
        effect.nullified = true;
        data = QVariant::fromValue(effect);
        return false;
    }
};

ThYejunCard::ThYejunCard() {
    m_skillName = "thyejunv";
    mute = true;
}

bool ThYejunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("thyejun")
            && to_select != Self && !to_select->hasFlag("ThYejunInvoked");
}

void ThYejunCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    if (target->hasLordSkill("thyejun")) {
        room->setPlayerFlag(target, "ThYejunInvoked");
        target->setChained(true);
        room->broadcastProperty(target, "chained");
        room->setEmotion(target, "chain");
        room->getThread()->trigger(ChainStateChanged, room, target);
        QList<ServerPlayer *> lords;
        QList<ServerPlayer *> players = room->getOtherPlayers(source);
        foreach (ServerPlayer *p, players) {
            if (p->hasLordSkill("thyejun") && !p->hasFlag("ThYejunInvoked")) {
                lords << p;
            }
        }
        if (lords.empty())
            room->setPlayerFlag(source, "ForbidThYejun");
    }
}

class ThYejunViewAsSkill: public ZeroCardViewAsSkill {
public:
    ThYejunViewAsSkill(): ZeroCardViewAsSkill("thyejunv"){
        attached_lord_skill = true;
    }

    virtual bool shouldBeVisible(const Player *player) const{
        return player->getKingdom() == "tsuki" && !player->hasFlag("ForbidThYejun");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if (Self->isChained())
            return false;
        foreach (const Player *p, Self->getAliveSiblings())
            if (p->isChained())
                return false;

        return player->getKingdom() == "tsuki" && !player->hasFlag("ForbidThYejun");
    }

    virtual const Card *viewAs() const{
        return new ThYejunCard;
    }
};

class ThYejun: public TriggerSkill{
public:
    ThYejun(): TriggerSkill("thyejun$") {
        events << GameStart << EventAcquireSkill << EventLoseSkill << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!player) return QStringList();
        if ((triggerEvent == GameStart && player->isLord())
            || (triggerEvent == EventAcquireSkill && data.toString() == "thyejun")) {
            QList<ServerPlayer *> lords;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasLordSkill(objectName()))
                    lords << p;
            }
            if (lords.isEmpty()) return QStringList();

            QList<ServerPlayer *> players;
            if (lords.length() > 1)
                players = room->getAlivePlayers();
            else
                players = room->getOtherPlayers(lords.first());
            foreach (ServerPlayer *p, players) {
                if (!p->hasSkill("thyejunv"))
                    room->attachSkillToPlayer(p, "thyejunv");
            }
        } else if (triggerEvent == EventLoseSkill && data.toString() == "thyejun") {
            QList<ServerPlayer *> lords;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasLordSkill(objectName()))
                    lords << p;
            }
            if (lords.length() > 2) return QStringList();

            QList<ServerPlayer *> players;
            if (lords.isEmpty())
                players = room->getAlivePlayers();
            else
                players << lords.first();
            foreach (ServerPlayer *p, players) {
                if (p->hasSkill("thyejunv"))
                    room->detachSkillFromPlayer(p, "thyejunv", true);
            }
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct phase_change = data.value<PhaseChangeStruct>();
            if (phase_change.from != Player::Play)
                  return QStringList();
            if (player->hasFlag("ForbidThYejun"))
                room->setPlayerFlag(player, "-ForbidThYejun");
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach (ServerPlayer *p, players) {
                if (p->hasFlag("ThYejunInvoked"))
                    room->setPlayerFlag(p, "-ThYejunInvoked");
            }
        }
        return QStringList();
    }
};

ThJinguoCard::ThJinguoCard() {
}

bool ThJinguoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (subcardsLength() == 0)
        return false;
    else
        return targets.isEmpty() && to_select != Self && !to_select->isNude();
}

bool ThJinguoCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    if (subcardsLength() == 0)
        return targets.isEmpty();
    else
        return !targets.isEmpty();
}

void ThJinguoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    if (subcardsLength() == 0) {
        room->setPlayerFlag(source, "thjinguo");
        room->acquireSkill(source, "thxueyi");
    } else {
        ServerPlayer *target = targets.first();
        room->setPlayerFlag(target, "thjinguo_InTempMoving");
        DummyCard *dummy = new DummyCard;
        QList<int> card_ids;
        QList<Player::Place> original_places;
        for (int i = 0; i < 2; i++) {
            if (target->isNude())
                break;
            card_ids << room->askForCardChosen(source, target, "he", objectName());
            original_places << room->getCardPlace(card_ids[i]);
            dummy->addSubcard(card_ids[i]);
            target->addToPile("#thjinguo", card_ids[i], false);
        }
        for (int i = 0; i < dummy->subcardsLength(); i++)
            room->moveCardTo(Sanguosha->getCard(card_ids[i]), target, original_places[i], false);
        room->setPlayerFlag(target, "-thjinguo_InTempMoving");
        if (dummy->subcardsLength() > 0)
            source->obtainCard(dummy, false);
        dummy->deleteLater();
        if (target->isWounded()) {
            RecoverStruct recover;
            recover.who = source;
            room->recover(target, recover);
        }
    }
}

class ThJinguoViewAsSkill: public ViewAsSkill {
public:
    ThJinguoViewAsSkill(): ViewAsSkill("thjinguo") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const {
        if (!selected.isEmpty()) return false;
        return !Self->isJilei(to_select) && to_select->getSuit() == Card::Heart && Self->getHandcards().contains(to_select);
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const {
        ThJinguoCard *card = new ThJinguoCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ThJinguoCard");
    }
};

class ThJinguo: public TriggerSkill {
public:
    ThJinguo(): TriggerSkill("thjinguo") {
        events << EventPhaseChanging;
        view_as_skill = new ThJinguoViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!player->hasFlag("thjinguo")) return QStringList();
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::NotActive)
            room->detachSkillFromPlayer(player, "thxueyi", false, true);
        return QStringList();
    }
};

class ThXueyi: public OneCardViewAsSkill {
public:
    ThXueyi(): OneCardViewAsSkill("thxueyi") {
        response_or_use = true;
        filter_pattern = "^TrickCard|heart";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isNude();
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        Indulgence *le = new Indulgence(originalCard->getSuit(), originalCard->getNumber());
        le->addSubcard(originalCard);
        le->setSkillName(objectName());
        return le;
    }
};

class ThLianmi: public TriggerSkill {
public:
    ThLianmi(): TriggerSkill("thlianmi") {
        events << EventPhaseStart;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        return TriggerSkill::triggerable(player)
            && player->getPhase() == Player::Start
            && player->getMark("@lianmi") <= 0
            && (player->getEquip(2) || player->getEquip(3));
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        room->addPlayerMark(player, "@lianmi");
        LogMessage log;
        log.type = "#ThLianmiWake";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);

        room->changeMaxHpForAwakenSkill(player);
        room->acquireSkill(player, "ikkuanggu");
        return false;
    }
};

class ThKuangqi: public TriggerSkill {
public:
    ThKuangqi(): TriggerSkill("thkuangqi") {
        events << DamageCaused;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Play)
            return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && (damage.card->isKindOf("Slash") || damage.card->isKindOf("Duel"))
            && !damage.transfer && !damage.chain && damage.by_user)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (room->askForCard(player, "Peach,Analeptic,EquipCard", "@thkuangqi", data, objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        DamageStruct damage = data.value<DamageStruct>();
        LogMessage log;
        log.type = "#ThKuangqi";
        log.from = player;
        log.to << damage.to;
        log.arg  = QString::number(damage.damage);
        log.arg2 = QString::number(++damage.damage);
        room->sendLog(log);

        data = QVariant::fromValue(damage);

        return false;
    }
};

class ThKaiyun: public TriggerSkill {
public:
    ThKaiyun(): TriggerSkill("thkaiyun") {
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        return TriggerSkill::triggerable(player) && player->canDiscard(player, "he");
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        JudgeStruct *judge = data.value<JudgeStruct *>();
        QStringList prompt_list;
        prompt_list << "@thkaiyun" << judge->who->objectName()
                    << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
        QString prompt = prompt_list.join(":");
        const Card *card = room->askForCard(player, ".." , prompt, data, objectName());
        if (card) {
            if (judge->who == player)
                room->broadcastSkillInvoke(objectName(), qrand() % 2 + 3);
            else
                room->broadcastSkillInvoke(objectName(), qrand() % 2 + 1);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        JudgeStruct *judge = data.value<JudgeStruct *>();
        QList<int> card_ids = room->getNCards(2, false);
        room->fillAG(card_ids, player);
        int card_id = room->askForAG(player, card_ids, false, objectName());
        card_ids.removeOne(card_id);
        room->clearAG(player);
        room->retrial(Sanguosha->getEngineCard(card_id), player, judge, objectName());

        room->obtainCard(player, card_ids.first(), false);
        return false;
    }
};

class ThJiaotu: public TriggerSkill {
public:
    ThJiaotu(): TriggerSkill("thjiaotu") {
        events << GameStart << Damaged << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == GameStart) {
            if (player) return QStringList();
            const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(Sanguosha->getSkill("ikwumou"));
            room->getThread()->addTriggerSkill(trigger_skill);
            return QStringList();
        }
        if (triggerEvent == Damaged && TriggerSkill::triggerable(player)) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.from)
                return QStringList();
            QStringList skills;
            for (int i = 0; i < damage.damage; i++)
                skills << objectName();
            return skills;
        } else if (triggerEvent == EventPhaseChanging && player->getMark("@jiaotu") > 0) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive)
                if (!player->hasFlag(objectName())) {
                    room->removePlayerMark(player, "@jiaotu");
                    room->detachSkillFromPlayer(player, "ikwumou", false, true);
                }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        DamageStruct damage = data.value<DamageStruct>();
        if (player->askForSkillInvoke(objectName(), QVariant::fromValue(damage.from))) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *) const {
        DamageStruct damage = data.value<DamageStruct>();
        JudgeStruct judge;
        judge.pattern = ".|heart";
        judge.good = true;
        judge.negative = true;
        judge.reason = objectName();
        judge.who = damage.from;
        room->judge(judge);

        if (judge.isBad() && damage.from->getMark("@jiaotu") <= 0) {
            if (room->getCurrent() == damage.from && damage.from->getPhase() != Player::NotActive)
                room->setPlayerFlag(damage.from, objectName());

            room->addPlayerMark(damage.from, "@jiaotu");
            room->acquireSkill(damage.from, "ikwumou");
        }
        return false;
    }
};

class ThShouye: public TriggerSkill {
public:
    ThShouye(): TriggerSkill("thshouye") {
        events << DrawNCards << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == DrawNCards && TriggerSkill::triggerable(player)) {
            if (data.toInt() > 0)
                return QStringList(objectName());
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive)
                foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
                    if (p->getMark("shouyetarget") > 0) {
                        room->setPlayerMark(p, "shouyetarget", 0);
                        Player::Phase origin_phase = p->getPhase();
                        p->setPhase(Player::Draw);
                        room->broadcastProperty(p, "phase");
                        RoomThread *thread = room->getThread();
                        if (!thread->trigger(EventPhaseStart, room, p))
                            thread->trigger(EventPhaseProceeding, room, p);
                        thread->trigger(EventPhaseEnd, room, p);

                        p->setPhase(origin_phase);
                        room->broadcastProperty(p, "phase");
                    }
                }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "@thshouye", true, true);
        if (target) {
            room->broadcastSkillInvoke(objectName());
            player->tag["ThShouyeTarget"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        ServerPlayer *target = player->tag["ThShouyeTarget"].value<ServerPlayer *>();
        player->tag.remove("ThShouyeTarget");
        if (target) {
            data = data.toInt() - 1;
            room->setPlayerMark(target, "shouyetarget", 1);
        }

        return false;
    }
};

ThXushiCard::ThXushiCard() {
    target_fixed = true;
}

const Card *ThXushiCard::validateInResponse(ServerPlayer *user) const {
    Room *room = user->getRoom();
    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "thxushi";
    room->sendLog(log);
    CardMoveReason reason(CardMoveReason::S_REASON_TURNOVER, user->objectName(), "thxushi", QString());
    const Card *card = Sanguosha->getCard(room->drawCard());
    room->moveCardTo(card, NULL, Player::PlaceTable, reason, true);
    if (card->isKindOf("Slash") || card->isKindOf("Jink"))
        user->obtainCard(card);
    else {
        CardMoveReason reason2(CardMoveReason::S_REASON_NATURAL_ENTER, user->objectName(), "thxushi", QString());
        room->throwCard(card, reason2, NULL);
    }
    room->setPlayerFlag(user, "Global_ThXushiFailed");
    return NULL;
}

const Card *ThXushiCard::validate(CardUseStruct &cardUse) const{
    ServerPlayer *user = cardUse.from;
    Room *room = user->getRoom();
    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "thxushi";
    room->sendLog(log);
    CardMoveReason reason(CardMoveReason::S_REASON_TURNOVER, user->objectName(), "thxushi", QString());
    const Card *card = Sanguosha->getCard(room->drawCard());
    room->moveCardTo(card, NULL, Player::PlaceTable, reason, true);
    if (card->isKindOf("Slash") || card->isKindOf("Jink"))
        user->obtainCard(card);
    else {
        CardMoveReason reason2(CardMoveReason::S_REASON_NATURAL_ENTER, user->objectName(), "thxushi", QString());
        room->throwCard(card, reason2, NULL);
    }
    room->setPlayerFlag(user, "Global_ThXushiFailed");
    return NULL;
}

class ThXushiViewAsSkill: public ZeroCardViewAsSkill {
public:
    ThXushiViewAsSkill(): ZeroCardViewAsSkill("thxushi") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if (player->getPhase() != Player::NotActive || player->hasFlag("Global_ThXushiFailed")) return false;
        if (pattern == "slash")
            return Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE;
        return false;
    }

    virtual const Card *viewAs() const{
        return new ThXushiCard;
    }
};

class ThXushi: public TriggerSkill {
public:
    ThXushi(): TriggerSkill("thxushi") {
        events << CardAsked;
        view_as_skill = new ThXushiViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player) || (player == room->getCurrent() && player->getPhase() != Player::NotActive))
            return QStringList();
        QString pattern = data.toStringList().first();
        if (pattern != "slash" && pattern != "jink")
            return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (!player->askForSkillInvoke(objectName()))
            return false;
        room->broadcastSkillInvoke(objectName());
        return true;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        CardMoveReason reason(CardMoveReason::S_REASON_TURNOVER, player->objectName(), objectName(), QString());
        const Card *card = Sanguosha->getCard(room->drawCard());
        room->moveCardTo(card, NULL, Player::PlaceTable, reason, true);
        if (card->isKindOf("Slash") || card->isKindOf("Jink"))
            player->obtainCard(card);
        else {
            CardMoveReason reason2(CardMoveReason::S_REASON_NATURAL_ENTER, player->objectName(), objectName(), QString());
            room->throwCard(card, reason2, NULL);
        }

        return false;
    }
};

class ThFengxiang: public TriggerSkill {
public:
    ThFengxiang(): TriggerSkill("thfengxiang") {
        events << ChainStateChanged;
        frequency = Frequent;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player->isDead()) return skill_list;
        foreach (ServerPlayer *p, room->findPlayersBySkillName(objectName()))
            skill_list.insert(p, QStringList(objectName()));
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const {
        if (ask_who->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const {
        ask_who->drawCards(1);
        return false;
    }
};

class ThKuaiqing: public TriggerSkill {
public:
    ThKuaiqing(): TriggerSkill("thkuaiqing") {
        events << TrickCardCanceling;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer* &ask_who) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if (effect.from && effect.from->isAlive() && effect.from->hasSkill(objectName())
            && !effect.to->isWounded() && effect.from != effect.to) {
                ask_who = effect.from;
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if (effect.from && effect.from->hasSkill(objectName()) && effect.from->isAlive()
            && !player->isWounded())
        {
            room->notifySkillInvoked(effect.from, objectName());
            return true;
        }

        return false;
    }
};

class ThYuhuo: public TriggerSkill {
public:
    ThYuhuo(): TriggerSkill("thyuhuo") {
        events << DamageInflicted;
        frequency = Limited;
        limit_mark = "@yuhuo";
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player) || player->getMark("@yuhuo") <= 0)
            return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from && damage.from->isAlive() && damage.from != player)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        room->removePlayerMark(player, "@yuhuo");
        room->addPlayerMark(player, "@yuhuoused");
        DamageStruct damage = data.value<DamageStruct>();
        damage.to = damage.from;
        damage.transfer = true;
        damage.transfer_reason = objectName();
        player->tag["TransferDamage"] = QVariant::fromValue(damage);
        if (damage.from->isAlive())
            damage.from->turnOver();
        return true;
    }
};

class ThCunjing: public TriggerSkill {
public:
    ThCunjing(): TriggerSkill("thcunjing") {
        events << SlashMissed;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player) || player->getWeapon() || !player->canDiscard(player, "he"))
            return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (room->askForCard(player, "..", "@thcunjing", data, objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        LogMessage log;
        log.type = "#ThCunjing";
        log.from = player;
        log.to << effect.to;
        log.arg = objectName();
        room->sendLog(log);
        room->slashResult(effect, NULL);
        return true;
    }
};

ThLianhuaCard::ThLianhuaCard() {
    target_fixed = true;
}

void ThLianhuaCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const {
    QList<int> pile_ids = room->getNCards(1, false);
    room->fillAG(pile_ids, source);
    ServerPlayer *target = room->askForPlayerChosen(source, room->getAllPlayers(), objectName());
    room->clearAG(source);

    DummyCard *dummy = new DummyCard(pile_ids);
    source->setFlags("Global_GongxinOperator");
    target->obtainCard(dummy, false);
    source->setFlags("-Global_GongxinOperator");
    delete dummy;
}

class ThLianhua: public OneCardViewAsSkill {
public:
    ThLianhua(): OneCardViewAsSkill("thlianhua") {
        filter_pattern = "EquipCard!";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *card = new ThLianhuaCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class ThQishu: public TriggerSkill {
public:
    ThQishu(): TriggerSkill("thqishu") {
        events << EventPhaseStart;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const {
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player->getPhase() == Player::NotActive)
            foreach (ServerPlayer *p, room->findPlayersBySkillName(objectName()))
                if (p != player && !p->faceUp())
                    skill_list.insert(p, QStringList(objectName()));
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const {
        if (ask_who->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const {
        ask_who->turnOver();
        ask_who->gainMark("@shiji");
        ask_who->gainAnExtraTurn();
        return false;
    }
};

class ThShiting: public TriggerSkill {
public:
    ThShiting(): TriggerSkill("thshiting") {
        events << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        return TriggerSkill::triggerable(player)
            && player->getPhase() == Player::Start
            && player->getMark("@shiji") <= 0;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        player->turnOver();
        if (player->getHandcardNum() < player->getMaxHp())
            player->drawCards(player->getMaxHp() - player->getHandcardNum());

        player->skip(Player::Judge);
        player->skip(Player::Draw);
        player->skip(Player::Play);
        player->skip(Player::Discard);
        return false;
    }
};

class ThHuanzai: public TriggerSkill {
public:
    ThHuanzai(): TriggerSkill("thhuanzai") {
        events << EventPhaseStart;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        return TriggerSkill::triggerable(player)
            && player->getPhase() == Player::Finish
            && player->getMark("@shiji") > 0;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(player, objectName());
        player->loseAllMarks("@shiji");
        return false;
    }
};

ThShennaoCard::ThShennaoCard(){
    target_fixed = true;
    will_throw = false;
}

const Card *ThShennaoCard::validate(CardUseStruct &cardUse) const {
    Nullification *use_card;
    if (subcardsLength() == 0) {
        ServerPlayer *player = cardUse.from;
        Room *room = player->getRoom();

        player->drawCards(1);
        room->loseHp(player);

        use_card = new Nullification(NoSuit, 0);
    } else {
        const Card *card = Sanguosha->getCard(getSubcards().first());
        use_card = new Nullification(card->getSuit(), card->getNumber());
        use_card->addSubcard(card);
    }
    use_card->setSkillName("thshennao");
    use_card->deleteLater();

    return use_card;
}

const Card *ThShennaoCard::validateInResponse(ServerPlayer *user) const {
    Nullification *use_card;
    if (subcardsLength() == 0) {
        Room *room = user->getRoom();

        user->drawCards(1);
        room->loseHp(user);

        use_card = new Nullification(NoSuit, 0);
    } else {
        const Card *card = Sanguosha->getCard(getSubcards().first());
        use_card = new Nullification(card->getSuit(), card->getNumber());
        use_card->addSubcard(card);
    }
    use_card->setSkillName("thshennao");
    use_card->deleteLater();

    return use_card;
}

class ThShennao: public ViewAsSkill {
public:
    ThShennao(): ViewAsSkill("thshennao") {
        response_pattern = "nullification";
        response_or_use = true;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const {
        if (Self->getHandcardNum() >= Self->getHp()) {
            return selected.isEmpty() && !to_select->isEquipped();
        } else {
            return false;
        }
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (Self->getHandcardNum() >= Self->getHp() && cards.isEmpty()) {
            return NULL;
        }
        ThShennaoCard *card = new ThShennaoCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        return player->isAlive();
    }
};

class ThMiaoyao: public TriggerSkill {
public:
    ThMiaoyao(): TriggerSkill("thmiaoyao") {
        events << CardResponded;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data,ServerPlayer* &) const{
        CardResponseStruct resp = data.value<CardResponseStruct>();
        if (!TriggerSkill::triggerable(player) || !resp.m_card->isKindOf("Jink") || player->getHp() != 1 || !player->isWounded())
            return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        RecoverStruct recover;
        recover.who = player;
        room->recover(player, recover);
        return false;
    }
};

ThHeiguanCard::ThHeiguanCard() {
    will_throw = false;
    handling_method = MethodNone;
}

bool ThHeiguanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty())
        return false;

    if (subcardsLength() == 0)
        return !to_select->isKongcheng() && to_select != Self;
    else
        return to_select != Self;
}

void ThHeiguanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const {
    ServerPlayer *target = targets.first();
    if (subcardsLength() == 0) {
        room->obtainCard(source,
                         room->askForCardChosen(source, target, "h", objectName()),
                         false);
        room->addPlayerMark(target, "@heiguan2");
    } else {
        CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), target->objectName(), "thheiguan", QString());
        room->obtainCard(target, this, reason);
        room->addPlayerMark(target, "@heiguan1");
    }
}

class ThHeiguanViewAsSkill: public ViewAsSkill {
public:
    ThHeiguanViewAsSkill(): ViewAsSkill("thheiguan") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.isEmpty() && to_select->isBlack() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        ThHeiguanCard *card = new ThHeiguanCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ThHeiguanCard");
    }
};

class ThHeiguan: public TriggerSkill {
public:
    ThHeiguan(): TriggerSkill("thheiguan") {
        events << EventPhaseStart << Death << EventLoseSkill;
        view_as_skill = new ThHeiguanViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (triggerEvent == EventPhaseStart || triggerEvent == Death) {
            if (triggerEvent == Death) {
                DeathStruct death = data.value<DeathStruct>();
                if (death.who != player)
                    return QStringList();
            }
            if (!player->hasSkill(objectName()))
                return QStringList();
            bool invoke = false;
            if ((triggerEvent == EventPhaseStart && player->getPhase() == Player::RoundStart) || triggerEvent == Death)
                invoke = true;
            if (!invoke)
                return QStringList();
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach (ServerPlayer *player, players) {
                room->setPlayerMark(player, "@heiguan1", 0);
                room->setPlayerMark(player, "@heiguan2", 0);
            }
        } else if (triggerEvent == EventLoseSkill && data.toString() == "thheiguan") {
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach (ServerPlayer *player, players) {
                room->setPlayerMark(player, "@heiguan1", 0);
                room->setPlayerMark(player, "@heiguan2", 0);
            }
        }

        return QStringList();
    }
};

class ThHeiguanProhibit: public ProhibitSkill {
public:
    ThHeiguanProhibit(): ProhibitSkill("#thheiguan-prohibit") {
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &) const{
        if (!card->isKindOf("Slash"))
            return false;
        if (to->hasSkill("thheiguan") && from->getMark("@heiguan1") > 0)
            return true;
        else if (to->getMark("@heiguan2") > 0)
            return true;
        else
            return false;
    }
};

class ThAnyue: public FilterSkill{
public:
    ThAnyue():FilterSkill("thanyue"){

    }

    static WrappedCard *changeToSpade(int cardId){
        WrappedCard *new_card = Sanguosha->getWrappedCard(cardId);
        new_card->setSkillName("thanyue");
        new_card->setSuit(Card::Spade);
        new_card->setModified(true);
        return new_card;
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        return changeToSpade(originalCard->getEffectiveId());
    }
};

class ThXiaoyong: public TriggerSkill {
public:
    ThXiaoyong(): TriggerSkill("thxiaoyong") {
        events << HpChanged;
        frequency = Frequent;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        JudgeStruct judge;
        judge.pattern = ".|heart";
        judge.good = false;
        judge.reason = objectName();
        judge.who = player;
        room->judge(judge);

        if (judge.isGood())
            player->drawCards(1);
        return false;
    }
};

ThKanyaoCard::ThKanyaoCard() {
    target_fixed = true;
}

void ThKanyaoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const {
    room->showAllCards(source);
    QMap<Suit, int> suits;
    foreach (const Card *card, source->getHandcards()) {
        Suit suit = card->getSuit();
        suits[suit] = suits.value(suit, 0) + 1;
    }
    bool same = false;
    foreach (Suit suit, suits.keys()) {
        if (suits[suit] > 0 && !same) {
            same = true;
            continue;
        }
        if (suits[suit] > 0 && same) {
            same = false;
            break;
        }
    }
    if (same) {
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(source))
            if (!p->isNude())
                targets << p;

        if (!targets.isEmpty()) {
            ServerPlayer *target = room->askForPlayerChosen(source, targets, "thkanyao", QString(), false, true);
            int card_id = room->askForCardChosen(source, target, "he", "thkanyao");
            CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, source->objectName());
            room->obtainCard(source, Sanguosha->getCard(card_id), reason, false);
        }
    } else {
        bool unsame = true;
        foreach (Suit suit, suits.keys())
            if (suits[suit] > 1) {
                unsame = false;
                break;
            }
        if (unsame) {
            QList<ServerPlayer *> victims;
            foreach (ServerPlayer *p, room->getOtherPlayers(source))
                if (p->getHp() >= source->getHp())
                    victims << p;
            if (!victims.isEmpty()) {
                ServerPlayer *victim = room->askForPlayerChosen(source, victims, "thkanyao", QString(), false, true);
                room->loseHp(victim);
            }
        }
    }
};

class ThKanyao: public ZeroCardViewAsSkill {
public:
    ThKanyao():ZeroCardViewAsSkill("thkanyao") {
    }

    virtual const Card *viewAs() const{
        return new ThKanyaoCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ThKanyaoCard") && player->getHandcardNum() >= player->getHp() && !player->isKongcheng();
    }
};

class ThZhehui: public TriggerSkill {
public:
    ThZhehui(): TriggerSkill("thzhehui") {
        events << Damaged << DamageInflicted << EventPhaseStart << EventLoseSkill;
        frequency = Compulsory;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        QMap<ServerPlayer *, QStringList> skill_list;
        if (triggerEvent == EventLoseSkill) {
            if (player->getMark("zhehui") > 0 && data.toString() == objectName())
                room->setPlayerMark(player, "zhehui", 0);
            return skill_list;
        }else if (triggerEvent == Damaged && TriggerSkill::triggerable(player) && player->getMark("zhehui") < 1)
            skill_list.insert(player, QStringList(objectName()));
        else if (triggerEvent == DamageInflicted && TriggerSkill::triggerable(player) && player->getMark("zhehui") > 0)
            skill_list.insert(player, QStringList(objectName()));
        else if (triggerEvent == EventPhaseStart)
            if (player->getPhase() == Player::NotActive)
                foreach (ServerPlayer *p, room->getAllPlayers())
                    room->setPlayerMark(p, "zhehui", 0);
            else if (player->getPhase() == Player::Finish)
                foreach (ServerPlayer *p, room->findPlayersBySkillName(objectName()))
                    if (p->getMark("zhehui") > 0)
                        skill_list.insert(p, QStringList(objectName()));
        return skill_list;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const {
        room->sendCompulsoryTriggerLog(ask_who, objectName());
        room->broadcastSkillInvoke(objectName());
        if (triggerEvent == Damaged)
            room->setPlayerMark(ask_who, "zhehui", 1);
        else if (triggerEvent == DamageInflicted)
            return true;
        else if (triggerEvent == EventPhaseStart) {
            room->setPlayerMark(ask_who, "zhehui", 0);
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, room->getOtherPlayers(ask_who))
                if (ask_who->canSlash(p, NULL, false))
                    targets << p;
            QStringList choices;
            if (ask_who->canDiscard(player, "he"))
                choices << "discard";
            if (Slash::IsAvailable(ask_who) && !targets.isEmpty())
                choices << "slash";
            if (choices.isEmpty()) return false;
            QString choice = room->askForChoice(ask_who, objectName(), choices.join("+"));
            if (choice == "slash") {
                ServerPlayer *victim = room->askForPlayerChosen(ask_who, targets, objectName(), "@dummy-slash");
                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName("_" + objectName());
                room->useCard(CardUseStruct(slash, ask_who, victim), false);
            } else {
                int card_ids = room->askForCardChosen(ask_who, player, "he", objectName(), false, Card::MethodDiscard);
                room->throwCard(card_ids, player, ask_who);
            }
        }
        return false;
    }
};

class ThChenji: public TriggerSkill {
public:
    ThChenji(): TriggerSkill("thchenji") {
        events << CardsMoveOneTime;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (TriggerSkill::triggerable(player) && (room->getCurrent() != player || player->getPhase() == Player::NotActive)
            && move.from == player && (move.from_places.contains(Player::PlaceHand) || move.from_places.contains(Player::PlaceEquip)))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName(), "@thchenji", true, true);
        if (target) {
            room->broadcastSkillInvoke(objectName());
            player->tag["ThChenjiTarget"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = player->tag["ThChenjiTarget"].value<ServerPlayer *>();
        player->tag.remove("ThChenjiTarget");
        if (target) {
            target->setChained(!target->isChained());
            room->broadcastProperty(target, "chained");
            room->setEmotion(target, "chain");
            room->getThread()->trigger(ChainStateChanged, room, target);
        }
        return false;
    }
};

class ThKuangxiang: public TriggerSkill {
public:
    ThKuangxiang(): TriggerSkill("thkuangxiang") {
        events << CardUsed;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        CardUseStruct use = data.value<CardUseStruct>();
        if (!(use.card->isNDTrick() || use.card->isKindOf("BasicCard")) || use.card->isKindOf("Jink") || use.card->isKindOf("Nullification"))
            return skill_list;
        if (use.to.isEmpty())
            use.to << use.from;
        //----------- 1
        foreach (ServerPlayer *to, use.to)
            if (to->isChained()) {
                foreach (ServerPlayer *owner, room->findPlayersBySkillName(objectName()))
                    if (!use.to.contains(owner))
                        skill_list.insert(owner, QStringList(objectName()));
                break;
            }
        //----------- 2
        foreach (ServerPlayer *owner, room->findPlayersBySkillName(objectName()))
            if (use.to.contains(owner))
                foreach (ServerPlayer *p, room->getAllPlayers())
                    if (p->isChained() && !use.to.contains(p)) {
                        skill_list.insert(owner, QStringList(objectName()));
                        break;
                    }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const {
        if (ask_who->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.to.isEmpty())
            use.to << use.from;
        if (!use.to.contains(ask_who)) {
            foreach (ServerPlayer *to, use.to)
                if (to->isChained())
                    room->addPlayerMark(to, "kuangxiang");
            use.to << ask_who;

            LogMessage log;
            log.type = "#ThYongyeAdd";
            log.from = ask_who;
            log.to << ask_who;
            log.arg = objectName();
            log.card_str = use.card->toString();
            room->sendLog(log);

            if (use.from) {
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, use.from->objectName(), ask_who->objectName());
                if (use.card->isKindOf("Collateral") && use.from->isAlive()) {
                    QList<ServerPlayer *> targets;
                    foreach (ServerPlayer *p, room->getOtherPlayers(ask_who))
                        if (ask_who->canSlash(p))
                            targets << p;
                    if (!targets.isEmpty()) {
                        ServerPlayer *target = room->askForPlayerChosen(use.from, targets, objectName(), "@dummy-slash2:" + ask_who->objectName());
                        if (!target)
                            target = targets.at(qrand() % targets.length());
                        ask_who->tag["collateralVictim"] = QVariant::fromValue(target);

                        LogMessage log;
                        log.type = "#CollateralSlash";
                        log.from = use.from;
                        log.to << target;
                        room->sendLog(log);
                        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, ask_who->objectName(), target->objectName());
                    } else {
                        ask_who->tag.remove("collateralVictim");
                        LogMessage log;
                        log.type = "#CollateralSlash";
                        log.from = ask_who;
                        room->sendLog(log);
                    }
                }
            }
        } else {
            foreach (ServerPlayer *p, room->getAllPlayers())
                if (p->isChained() && !use.to.contains(p)) {
                    room->addPlayerMark(p, "kuangxiang");
                    use.to << p;

                    LogMessage log;
                    log.type = "#ThYongyeAdd";
                    log.from = ask_who;
                    log.to << p;
                    log.arg = objectName();
                    log.card_str = use.card->toString();
                    room->sendLog(log);

                    if (use.from) {
                        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, use.from->objectName(), p->objectName());
                        if (use.card->isKindOf("Collateral") && use.from->isAlive()) {
                            QList<ServerPlayer *> targets;
                            foreach (ServerPlayer *player, room->getOtherPlayers(p))
                                if (p->canSlash(player))
                                    targets << player;
                            if (!targets.isEmpty()) {
                                ServerPlayer *target = room->askForPlayerChosen(use.from, targets, objectName(), "@dummy-slash2:" + p->objectName());
                                if (!target)
                                    target = targets.at(qrand() % targets.length());
                                p->tag["collateralVictim"] = QVariant::fromValue(target);

                                LogMessage log;
                                log.type = "#CollateralSlash";
                                log.from = use.from;
                                log.to << target;
                                room->sendLog(log);
                                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, p->objectName(), target->objectName());
                            } else {
                                p->tag.remove("collateralVictim");

                                LogMessage log;
                                log.type = "#CollateralNoSlash";
                                log.from = p;
                                room->sendLog(log);
                            }
                        }
                    }
                }
        }

        room->setTag("ThKuangxiangCard", QVariant::fromValue(use.card->toString()));
        room->sortByActionOrder(use.to);
        data = QVariant::fromValue(use);
        return false;
    }
};

class ThKuangxiangClear: public TriggerSkill {
public:
    ThKuangxiangClear(): TriggerSkill("#thkuangxiang") {
        events << CardFinished;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer* &) const{
        QString cardstr = room->getTag("ThKuangxiangCard").toString();
        if (data.value<CardUseStruct>().card->toString() == cardstr) {
            room->removeTag("ThKuangxiangCard");
            foreach (ServerPlayer *p, room->getAllPlayers())
                if (p->getMark("kuangxiang") > 0) {
                    room->setPlayerMark(p, "kuangxiang", 0);
                    if (p->isChained()) {
                        room->setEmotion(p, "chain");
                        room->setPlayerProperty(p, "chained", false);
                    }
                }
        }

        return QStringList();
    }
};

ThExiCard::ThExiCard(){
}

bool ThExiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (targets.length() >= 2)
        return false;

    if (to_select == Self)
        return Self->getHandcardNum() > 1;
    else
        return !to_select->isKongcheng();
}

bool ThExiCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return targets.length() == 2;
}

void ThExiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    QList<const Card *> cards;
    foreach (ServerPlayer *p, targets)
        cards << room->askForCardShow(p, source, "thexi");

    room->showCard(targets[0], cards[0]->getId());
    room->showCard(targets[1], cards[1]->getId());
    const Card *big, *small;
    ServerPlayer *target;
    if (cards[0]->getNumber() == cards[1]->getNumber())
        source->addMark("exi");
    else {
        if (cards[0]->getNumber() > cards[1]->getNumber()) {
            big = cards[0];
            small = cards[1];
            target = targets[0];
        } else {
            big = cards[1];
            small = cards[0];
            target = targets[1];
        }

        if (target == source) {
            source->obtainCard(big);
            source->obtainCard(small);
        } else if (room->askForChoice(source, "thexi", "big+small") == "big") {
            source->obtainCard(big);
            target->obtainCard(small);
        } else {
            source->obtainCard(small);
            target->obtainCard(big);
        }
    }
}

class ThExiViewAsSkill: public OneCardViewAsSkill {
public:
    ThExiViewAsSkill(): OneCardViewAsSkill("thexi") {
        response_pattern = "@@thexi";
        filter_pattern = ".|.|.|hand";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *card = new ThExiCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class ThExi: public TriggerSkill {
public:
    ThExi(): TriggerSkill("thexi") {
        events << EventPhaseStart;
        view_as_skill = new ThExiViewAsSkill;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        if (player->getPhase() == Player::NotActive) {
            foreach (ServerPlayer *p, room->getAlivePlayers())
                if (p->getMark("exi") > 0)
                    skill_list.insert(p, QStringList(objectName()));
        } else if (player->getPhase() == Player::Finish && TriggerSkill::triggerable(player) && player->getMark("exicount") <= 0
            && player->canDiscard(player, "h")) {
            int n = 0;
            foreach (ServerPlayer *p, room->getAllPlayers()) {
                if (p == player && p->getHandcardNum() > 1)
                    n++;
                else if (p != player && !p->isKongcheng())
                    n++;
                if (n > 1) {
                    skill_list.insert(player, QStringList(objectName()));
                    break;
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const {
        if (player->getPhase() == Player::NotActive)
            return true;
        room->askForUseCard(ask_who, "@@thexi", "@thexi");
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const {
        room->removePlayerMark(ask_who, "exi");
        room->addPlayerMark(ask_who, "exicount");
        ask_who->gainAnExtraTurn();
        room->removePlayerMark(ask_who, "exicount");
        return false;
    }
};

class ThXinglu: public TriggerSkill {
public:
    ThXinglu(): TriggerSkill("thxinglu") {
        events << Dying;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        DyingStruct dying = data.value<DyingStruct>();
        if (!TriggerSkill::triggerable(player) || dying.who != player
            || player->isKongcheng() || player->getHp() > 0)
            return QStringList();
        foreach (ServerPlayer *p, room->getOtherPlayers(player))
            if (!p->isKongcheng())
                return QStringList(objectName());

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(player))
            if (!p->isKongcheng())
                targets << p;
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@thxinglu", true, true);
        if (target) {
            room->broadcastSkillInvoke(objectName());
            player->tag["ThXingluTarget"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = player->tag["ThXingluTarget"].value<ServerPlayer *>();
        player->tag.remove("ThXingluTarget");
        if (target) {
            bool win = player->pindian(target, objectName());
            if (win) {
                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
            }
        }
        return false;
    }
};

class ThAnbing: public TriggerSkill {
public:
    ThAnbing(): TriggerSkill("thanbing") {
        events << PreCardUsed << EventPhaseChanging;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::Discard && TriggerSkill::triggerable(player) && !player->hasFlag("ThAnbingTrickInPlayPhase")) {
                return QStringList(objectName());
            }
        } else if (player->getPhase() == Player::Play) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("TrickCard"))
                room->setPlayerFlag(player, "ThAnbingTrickInPlayPhase");
        }

        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(player, objectName());
        player->skip(Player::Discard);
        return false;
    }
};

class ThAnbingMaxCardsSkill: public MaxCardsSkill {
public:
    ThAnbingMaxCardsSkill(): MaxCardsSkill("#thanbing") {
    }

    virtual int getExtra(const Player *target) const{
        if (target->hasSkill("thanbing"))
            return -1;
        else
            return 0;
    }
};

class ThHuilveViewAsSkill: public OneCardViewAsSkill {
public:
    ThHuilveViewAsSkill(): OneCardViewAsSkill("thhuilve") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        QStringList card_str = player->property("thhuilve").toString().split("+");
        if (card_str.size() < 2) return false;
        Card *trick = Sanguosha->cloneCard(card_str.first(), (Card::Suit)card_str.last().toInt());
        trick->setSkillName(objectName());
        trick->deleteLater();
        return trick->isAvailable(player);
    }

    virtual bool viewFilter(const Card *card) const{
        QStringList card_str = Self->property("thhuilve").toString().split("+");
        if (card_str.size() < 2) return false;
        return !card->isEquipped() && card->getSuit() == (Card::Suit)card_str.last().toInt();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        QStringList card_str = Self->property("thhuilve").toString().split("+");
        if (card_str.size() < 2) return false;
        Card *trick = Sanguosha->cloneCard(card_str.first(), originalCard->getSuit(), originalCard->getNumber());
        trick->addSubcard(originalCard);
        trick->setSkillName(objectName());
        return trick;
    }
};

class ThHuilve: public TriggerSkill {
public:
    ThHuilve(): TriggerSkill("thhuilve") {
        events << PreCardUsed << EventPhaseChanging;
        view_as_skill = new ThHuilveViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == PreCardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isNDTrick() || use.card->isKindOf("Nullification"))
                return QStringList();

            QString skill_name = use.card->getSkillName();
            bool changed = false;
            if (!skill_name.isEmpty()) {
                const ViewAsSkill *skill = Sanguosha->getViewAsSkill(skill_name);
                if (skill && !skill->inherits("FilterSkill"))
                    changed = true;
            }
            if (changed)
                room->setPlayerProperty(player, "thhuilve", "");
            else {
                QStringList str;
                str << use.card->objectName() << QString::number((int)use.card->getSuit());
                room->setPlayerProperty(player, "thhuilve", str.join("+"));
            }
        } else
            room->setPlayerProperty(player, "thhuilve", "");

        return QStringList();
    }
};

class ThJizhi: public TargetModSkill {
public:
    ThJizhi(): TargetModSkill("thjizhi") {
        pattern = "TrickCard";
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill(objectName()))
            return 1000;
        else
            return 0;
    }
};

class ThShenyou: public TriggerSkill {
public:
    ThShenyou(): TriggerSkill("thshenyou") {
        events << EventPhaseStart << Predamage;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (triggerEvent == EventPhaseStart && player->getPhase() == Player::Draw && TriggerSkill::triggerable(player)) {
            foreach (ServerPlayer *p, room->getOtherPlayers(player))
                if (!p->isAllNude())
                    return QStringList(objectName());
        } else if (triggerEvent == Predamage) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->isKindOf("Slash")
                && player->getMark(objectName()) > 0)
                return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (triggerEvent == EventPhaseStart) {
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, room->getOtherPlayers(player))
                if (!p->isAllNude())
                    targets << p;
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@thshenyou-invoke", true, true);
            if (target) {
                room->broadcastSkillInvoke(objectName());
                player->tag["ThShenyouTarget"] = QVariant::fromValue(target);
                return true;
            }
        } else if (triggerEvent == Predamage) {
            if (player->askForSkillInvoke(objectName(), "losehp"))
                return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (triggerEvent == EventPhaseStart) {
            ServerPlayer *target = player->tag["ThShenyouTarget"].value<ServerPlayer *>();
            player->tag.remove("ThShenyouTarget");
            if (target) {
                int card_id = room->askForCardChosen(player, target, "hej", objectName());
                room->obtainCard(player, card_id, false);
                room->addPlayerMark(player, objectName());
                room->askForUseSlashTo(player, target, "@thshenyou:" + target->objectName(), false);
                room->removePlayerMark(player, objectName());
                return true;
            }
        } else if (triggerEvent == Predamage) {
            DamageStruct damage = data.value<DamageStruct>();
            room->loseHp(damage.to, damage.damage);
            return true;
        }
        return false;
    }
};

class ThGuixu: public TriggerSkill {
public:
    ThGuixu(): TriggerSkill("thguixu") {
        events << CardUsed;
    }

    virtual QMap<ServerPlayer *, QStringList> triggerable(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        QMap<ServerPlayer *, QStringList> skill_list;
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("TrickCard") && !use.card->isKindOf("Nullification")) {
            foreach (ServerPlayer *owner, room->findPlayersBySkillName(objectName()))
                if (owner->getPile("guixupile").length() <= 1 && (owner != room->getCurrent() || owner->getPhase() == Player::NotActive))
                    skill_list.insert(owner, QStringList(objectName()));
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const {
        if (ask_who->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const {
        JudgeStruct judge;
        judge.pattern = ".|red";
        judge.good = true;
        judge.reason = objectName();
        judge.who = ask_who;
        room->judge(judge);

        if (judge.isGood()) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (room->getCardPlace(use.card->getEffectiveId()) == Player::PlaceTable) {
                ask_who->addToPile("guixupile", use.card);
                foreach (ServerPlayer *to, use.to)
                    use.nullified_list << to->objectName();
                data = QVariant::fromValue(use);
            } else if (room->getCardPlace(use.card->getEffectiveId()) == Player::PlaceDelayedTrick) {
                ask_who->addToPile("guixupile", use.card);
            }
        }
        return false;
    }
};

ThTianqueCard::ThTianqueCard() {
    will_throw = false;
    target_fixed = true;
    handling_method = MethodNone;
}

void ThTianqueCard::onUse(Room *room, const CardUseStruct &use) const {
    CardUseStruct card_use = use;
    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();
    thread->trigger(PreCardUsed, room, card_use.from, data);
    card_use = data.value<CardUseStruct>();
    QList<int> card_ids = card_use.card->getSubcards();
    LogMessage log;
    log.type = "$DiscardCardWithSkill";
    log.from = card_use.from;
    log.arg = "thtianque";
    log.card_str = QString::number(card_ids.first());
    room->sendLog(log);

    int to_get = card_ids.takeLast();
    DummyCard *dummy = new DummyCard(card_ids);
    dummy->deleteLater();
    CardMoveReason reason(CardMoveReason::S_REASON_THROW, card_use.from->objectName(), QString(), card_use.card->getSkillName(), QString());
    room->moveCardTo(dummy, card_use.from, NULL, Player::DiscardPile, reason, true);
    thread->trigger(CardUsed, room, card_use.from, data);
    card_use = data.value<CardUseStruct>();
    thread->trigger(CardFinished, room, card_use.from, data);
    room->obtainCard(card_use.from, to_get);
}

class ThTianque: public ViewAsSkill {
public:
    ThTianque(): ViewAsSkill("thtianque") {
        expand_pile = "guixupile";
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return !player->isKongcheng() && !player->getPile("guixupile").isEmpty();
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const {
        if (selected.isEmpty()) {
            if (Self->isJilei(to_select) || !Self->getHandcards().contains(to_select))
                return false;
            foreach (int id, Self->getPile("guixupile"))
                if (Sanguosha->getCard(id)->getSuit() == to_select->getSuit())
                    return true;
            return false;
        } else if (selected.length() == 1) {
            if (!Self->getPile("guixupile").contains(to_select->getEffectiveId()))
                return false;
            return to_select->getSuit() == selected.first()->getSuit();
        } else
            return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const {
        if (cards.size() == 2) {
            ThTianqueCard *card = new ThTianqueCard;
            card->addSubcards(cards);
            return card;
        } else
            return NULL;
    }
};

#include "ikai-kin.h"
class ThYongyeViewAsSkill: public ZeroCardViewAsSkill {
public:
    ThYongyeViewAsSkill(): ZeroCardViewAsSkill("thyongye") {
        response_pattern = "@@thyongye!";
    }

    virtual const Card *viewAs() const{
        return new ExtraCollateralCard;
    }
};

class ThYongye: public TriggerSkill {
public:
    ThYongye(): TriggerSkill("thyongye") {
        events << CardUsed;
        view_as_skill = new ThYongyeViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        CardUseStruct use = data.value<CardUseStruct>();
        if (TriggerSkill::triggerable(player) && use.card->isNDTrick()) {
            if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_PLAY)
                return QStringList();
            QList<ServerPlayer *> available_targets;
            if (!use.card->isKindOf("AOE") && !use.card->isKindOf("GlobalEffect")) {
                foreach (ServerPlayer *p, room->getAlivePlayers()) {
                    if (use.to.contains(p) || room->isProhibited(player, p, use.card)) continue;
                    if (use.card->targetFixed()) {
                        available_targets << p;
                    } else {
                        if (use.card->targetFilter(QList<const Player *>(), p, player))
                            available_targets << p;
                    }
                }
            }
            QStringList choices;
            if ((use.to.length() > 1 || !available_targets.isEmpty()) && !player->isKongcheng())
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (!room->askForCard(player, ".|black", "@thyongye", QVariant(), objectName())) return false;
        room->broadcastSkillInvoke(objectName());
        return true;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        CardUseStruct use = data.value<CardUseStruct>();
        QList<ServerPlayer *> available_targets;
        if (!use.card->isKindOf("AOE") && !use.card->isKindOf("GlobalEffect")) {
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (use.to.contains(p) || room->isProhibited(player, p, use.card)) continue;
                if (use.card->targetFixed()) {
                    available_targets << p;
                } else {
                    if (use.card->targetFilter(QList<const Player *>(), p, player))
                        available_targets << p;
                }
            }
        }
        QStringList choices;
        if (use.to.length() > 1) choices.prepend("remove");
        if (!available_targets.isEmpty()) choices.prepend("add");
        QString choice = room->askForChoice(player, "thyongye", choices.join("+"), data);
        if (choice == "add") {
            ServerPlayer *extra = NULL;
            if (!use.card->isKindOf("Collateral"))
                extra = room->askForPlayerChosen(player, available_targets, "thyongye", "@thyongye-add:::" + use.card->objectName());
            else {
                QStringList tos;
                foreach (ServerPlayer *t, use.to)
                    tos.append(t->objectName());
                room->setPlayerProperty(player, "extra_collateral", use.card->toString());
                room->setPlayerProperty(player, "extra_collateral_current_targets", tos.join("+"));
                room->askForUseCard(player, "@@thyongye!", "@thyongye-add:::collateral");
                room->setPlayerProperty(player, "extra_collateral", QString());
                room->setPlayerProperty(player, "extra_collateral_current_targets", QString("+"));
                foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                    if (p->hasFlag("ExtraCollateralTarget")) {
                        p->setFlags("-ExtraCollateralTarget");
                        extra = p;
                        break;
                    }
                }
                if (extra == NULL) {
                    extra = available_targets.at(qrand() % available_targets.length() - 1);
                    QList<ServerPlayer *> victims;
                    foreach (ServerPlayer *p, room->getOtherPlayers(extra)) {
                        if (extra->canSlash(p)
                            && (!(p == player && p->hasSkill("ikjingyou") && p->isLastHandCard(use.card, true)))) {
                            victims << p;
                        }
                    }
                    Q_ASSERT(!victims.isEmpty());
                    extra->tag["collateralVictim"] = QVariant::fromValue(victims.at(qrand() % victims.length()));
                }
            }
            use.to.append(extra);
            room->sortByActionOrder(use.to);

            LogMessage log;
            log.type = "#ThYongyeAdd";
            log.from = player;
            log.to << extra;
            log.card_str = use.card->toString();
            log.arg = "thyongye";
            room->sendLog(log);
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), extra->objectName());

            if (use.card->isKindOf("Collateral")) {
                ServerPlayer *victim = extra->tag["collateralVictim"].value<ServerPlayer *>();
                if (victim) {
                    LogMessage log;
                    log.type = "#CollateralSlash";
                    log.from = player;
                    log.to << victim;
                    room->sendLog(log);
                    room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, extra->objectName(), victim->objectName());
                }
            }
        } else if (choice == "remove") {
            ServerPlayer *removed = room->askForPlayerChosen(player, use.to, "thyongye", "@thyongye-remove:::" + use.card->objectName());
            use.to.removeOne(removed);

            LogMessage log;
            log.type = "#ThYongyeRemove";
            log.from = player;
            log.to << removed;
            log.card_str = use.card->toString();
            log.arg = "thyongye";
            room->sendLog(log);
        }
        data = QVariant::fromValue(use);

        return false;
    }
};

class ThShiming: public TriggerSkill {
public:
    ThShiming(): TriggerSkill("thshiming") {
        events << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        return TriggerSkill::triggerable(player)
            && player->getPhase() == Player::Draw;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        QList<int> card_ids = room->getNCards(4, false);
        CardMoveReason reason(CardMoveReason::S_REASON_TURNOVER, player->objectName(), objectName(), QString());
        room->moveCardsAtomic(CardsMoveStruct(card_ids, NULL, Player::PlaceTable, reason), true);
        QList<int> red_to_get, red_to_disc, nine_to_get, nine_to_disc;
        foreach (int id, card_ids) {
            if (Sanguosha->getCard(id)->isRed())
                red_to_get << id;
            else
                red_to_disc << id;
            if (Sanguosha->getCard(id)->getNumber() <= 9)
                nine_to_get << id;
            else
                nine_to_disc << id;
        }
        QStringList choices;
        if (!red_to_get.isEmpty())
            choices << "red";
        if (!nine_to_get.isEmpty())
            choices << "nine";

        QString choice = "";
        if (!choices.isEmpty())
            choice = room->askForChoice(player, objectName(), choices.join("+"));
        DummyCard *dummy = new DummyCard;
        if (choice == "red") {
            dummy->addSubcards(red_to_get);
            player->obtainCard(dummy);
            dummy->clearSubcards();
            if (!red_to_disc.isEmpty()) {
                dummy->addSubcards(red_to_disc);
                CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, player->objectName(), objectName(), QString());
                room->throwCard(dummy, reason, NULL);
            }
        } else if (choice == "nine") {
            dummy->addSubcards(nine_to_get);
            player->obtainCard(dummy);
            dummy->clearSubcards();
            if (!nine_to_disc.isEmpty()) {
                dummy->addSubcards(nine_to_disc);
                CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, player->objectName(), objectName(), QString());
                room->throwCard(dummy, reason, NULL);
            }
        } else {
            dummy->addSubcards(card_ids);
            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, player->objectName(), objectName(), QString());
            room->throwCard(dummy, reason, NULL);
        }

        delete dummy;
        return true;
    }
};

ThShenbaoCard::ThShenbaoCard() {
}

bool ThShenbaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    return targets.isEmpty() && !to_select->isAllNude() && to_select != Self;
}

void ThShenbaoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const {
    ServerPlayer *target = targets.first();
    if (target->isAllNude())
        return ;

    room->removePlayerMark(source, "@shenbao");
    room->addPlayerMark(source, "@shenbaoused");
    room->setPlayerFlag(target, "thshenbao_InTempMoving");
    DummyCard *dummy = new DummyCard;
    int n = qMin(source->getAttackRange(), 3);
    QList<int> card_ids;
    QList<Player::Place> original_places;
    for (int i = 0; i < n; i++) {
        if (target->isAllNude())
            break;
        card_ids << room->askForCardChosen(source, target, "hej", objectName());
        original_places << room->getCardPlace(card_ids[i]);
        dummy->addSubcard(card_ids[i]);
        target->addToPile("#thshenbao", card_ids[i], false);
    }
    for (int i = 0; i < dummy->subcardsLength(); i++)
        room->moveCardTo(Sanguosha->getCard(card_ids[i]), target, original_places[i], false);
    room->setPlayerFlag(target, "-thshenbao_InTempMoving");
    if (dummy->subcardsLength() > 0)
        room->obtainCard(source, dummy, false);
    dummy->deleteLater();
    room->setPlayerFlag(source, "shenbaoused");
}

class ThShenbaoViewAsSkill: public ZeroCardViewAsSkill {
public:
    ThShenbaoViewAsSkill(): ZeroCardViewAsSkill("thshenbao") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@shenbao") > 0;
    }

    virtual const Card *viewAs() const{
        return new ThShenbaoCard;
    }
};

class ThShenbao: public TriggerSkill {
public:
    ThShenbao(): TriggerSkill("thshenbao") {
        events << EventPhaseStart;
        view_as_skill = new ThShenbaoViewAsSkill;
        frequency = Limited;
        limit_mark = "@shenbao";
    }

    virtual bool triggerable(const ServerPlayer *target) const {
        return TriggerSkill::triggerable(target)
            && target->hasFlag("shenbaoused")
            && !target->isKongcheng()
            && target->getPhase() == Player::Finish;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        room->sendCompulsoryTriggerLog(player, objectName());
        int n = player->getAttackRange();
        room->askForDiscard(player, objectName(), n, n, false, true);
        return false;
    }
};

class ThYunyin: public TriggerSkill {
public:
    ThYunyin(): TriggerSkill("thyunyin$") {
        events << Damage;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const {
        if (player->tag.value("InvokeThYunyin", false).toBool()) {
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p->hasLordSkill(objectName()))
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (p->hasLordSkill("thyunyin"))
                targets << p;
        }

        while (!targets.isEmpty()) {
            ServerPlayer *target = room->askForPlayerChosen(player, targets, "thyunyin", QString(), true);
            if (target) {
                targets.removeOne(target);

                LogMessage log;
                log.type = "#InvokeOthersSkill";
                log.from = player;
                log.to << target;
                log.arg = objectName();
                room->sendLog(log);
                room->notifySkillInvoked(target, objectName());

                JudgeStruct judge;
                judge.pattern = ".|black";
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;

                room->judge(judge);

                if (judge.isGood()) {
                    QList<ServerPlayer *> victims;
                    foreach (ServerPlayer *p, room->getAllPlayers())
                        if (p->getWeapon())
                            victims << p;

                    if (!victims.isEmpty()) {
                        ServerPlayer *victim = room->askForPlayerChosen(player, victims, objectName());
                        target->obtainCard(victim->getWeapon());
                    }
                }
            } else
                break;
        }
        return false;
    }
};

class ThYunyinRecord: public TriggerSkill {
public:
    ThYunyinRecord(): TriggerSkill("#thyunyin-record") {
        events << PreDamageDone;
        frequency = Compulsory;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &data, ServerPlayer* &) const {
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *yilun = damage.from;
        if (yilun)
            yilun->tag["InvokeThYunyin"] = yilun->getKingdom() == "tsuki";
        return QStringList();
    }
};

TouhouTsukiPackage::TouhouTsukiPackage()
    :Package("touhou-tsuki")
{
    General *tsuki001 = new General(this, "tsuki001$", "tsuki");
    tsuki001->addSkill(new ThSuoming);
    tsuki001->addSkill(new ThChiwu);
    tsuki001->addSkill(new ThYejun);

    General *tsuki002 = new General(this, "tsuki002", "tsuki");
    tsuki002->addSkill(new ThJinguo);
    tsuki002->addSkill(new FakeMoveSkill("thjinguo"));
    related_skills.insertMulti("thjinguo", "#thjinguo-fake-move");
    tsuki002->addSkill(new ThLianmi);
    tsuki002->addRelateSkill("thxueyi");

    General *tsuki003 = new General(this, "tsuki003", "tsuki");
    tsuki003->addSkill(new ThKuangqi);

    General *tsuki004 = new General(this, "tsuki004", "tsuki", 3, false);
    tsuki004->addSkill(new ThKaiyun);
    tsuki004->addSkill(new ThJiaotu);

    General *tsuki005 = new General(this, "tsuki005", "tsuki", 3);
    tsuki005->addSkill(new ThShouye);
    tsuki005->addSkill(new ThXushi);

    General *tsuki006 = new General(this, "tsuki006", "tsuki", 3);
    tsuki006->addSkill(new ThFengxiang);
    tsuki006->addSkill(new ThKuaiqing);
    tsuki006->addSkill(new ThYuhuo);

    General *tsuki007 = new General(this, "tsuki007", "tsuki");
    tsuki007->addSkill(new ThCunjing);
    tsuki007->addSkill(new ThLianhua);

    General *tsuki008 = new General(this, "tsuki008", "tsuki", 3);
    tsuki008->addSkill(new ThQishu);
    tsuki008->addSkill(new ThShiting);
    tsuki008->addSkill(new ThHuanzai);

    General *tsuki009 = new General(this, "tsuki009", "tsuki", 3);
    tsuki009->addSkill(new ThShennao);
    tsuki009->addSkill(new ThMiaoyao);

    General *tsuki010 = new General(this, "tsuki010", "tsuki");
    tsuki010->addSkill(new ThHeiguan);
    tsuki010->addSkill(new ThHeiguanProhibit);
    tsuki010->addSkill(new ThAnyue);
    related_skills.insertMulti("thheiguan", "#thheiguan-prohibit");

    General *tsuki011 = new General(this, "tsuki011", "tsuki", 3);
    tsuki011->addSkill(new ThXiaoyong);
    tsuki011->addSkill(new ThKanyao);

    General *tsuki012 = new General(this, "tsuki012", "tsuki");
    tsuki012->addSkill(new ThZhehui);
    tsuki012->addSkill(new SlashNoDistanceLimitSkill("thzhehui"));
    related_skills.insertMulti("thzhehui", "#thzhehui-slash-ndl");

    General *tsuki013 = new General(this, "tsuki013", "tsuki", 3);
    tsuki013->addSkill(new ThChenji);
    tsuki013->addSkill(new ThKuangxiang);
    tsuki013->addSkill(new ThKuangxiangClear);
    related_skills.insertMulti("thkuangxiang", "#thkuangxiang");

    General *tsuki014 = new General(this, "tsuki014", "tsuki", 3);
    tsuki014->addSkill(new ThExi);
    tsuki014->addSkill(new ThXinglu);

    General *tsuki015 = new General(this, "tsuki015", "tsuki", 3, false);
    tsuki015->addSkill(new ThAnbing);
    tsuki015->addSkill(new ThAnbingMaxCardsSkill);
    related_skills.insertMulti("thanbing", "#thanbing");
    tsuki015->addSkill(new ThHuilve);
    tsuki015->addSkill(new ThJizhi);

    General *tsuki016 = new General(this, "tsuki016", "tsuki");
    tsuki016->addSkill(new ThShenyou);

    General *tsuki017 = new General(this, "tsuki017", "tsuki");
    tsuki017->addSkill(new ThGuixu);
    tsuki017->addSkill(new ThTianque);

    General *tsuki018 = new General(this, "tsuki018$", "tsuki", 3);
    tsuki018->addSkill(new ThYongye);
    tsuki018->addSkill(new ThShiming);
    tsuki018->addSkill(new ThShenbao);
    tsuki002->addSkill(new FakeMoveSkill("thshenbao"));
    related_skills.insertMulti("thshenbao", "#thshenbao-fake-move");
    tsuki018->addSkill(new ThYunyin);
    tsuki018->addSkill(new ThYunyinRecord);
    related_skills.insertMulti("thyunyin", "#thyunyin-record");

    addMetaObject<ThYejunCard>();
    addMetaObject<ThJinguoCard>();
    addMetaObject<ThXushiCard>();
    addMetaObject<ThLianhuaCard>();
    addMetaObject<ThShennaoCard>();
    addMetaObject<ThHeiguanCard>();
    addMetaObject<ThKanyaoCard>();
    addMetaObject<ThExiCard>();
    addMetaObject<ThTianqueCard>();
    addMetaObject<ThShenbaoCard>();

    skills << new ThYejunViewAsSkill << new ThXueyi;
}

ADD_PACKAGE(TouhouTsuki)