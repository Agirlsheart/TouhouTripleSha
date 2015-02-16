#include "touhou-hana.h"

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

class ThHuaji: public TriggerSkill {
public:
    ThHuaji(): TriggerSkill("thhuaji") {
        events << CardUsed << CardResponded;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
        if (triggerEvent == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            ServerPlayer *current = room->getCurrent();
            if (TriggerSkill::triggerable(current) && current->getPhase() != Player::NotActive && player != current
                && (use.card->isNDTrick() || use.card->isKindOf("BasicCard"))) {
                ask_who = current;
                return QStringList(objectName());
            }
        } else if (triggerEvent == CardResponded) {
            CardResponseStruct resp = data.value<CardResponseStruct>();
            ServerPlayer *current = room->getCurrent();
            if (TriggerSkill::triggerable(current) && current->getPhase() != Player::NotActive && player != current
                && resp.m_isUse && (resp.m_card->isNDTrick() || resp.m_card->isKindOf("BasicCard"))) {
                ask_who = current;
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
        //if (data.canConvert<CardUseStruct>())
        //    ask_who->tag["thhuajiTarget"] = QVariant::fromValue(data.value<CardUseStruct>().from);
        //else if (data.canConvert<CardResponseStruct>())
        //    ask_who->tag["thhuajiTarget"] = QVariant::fromValue(player);
        ask_who->tag["thhuajiTarget"] = QVariant::fromValue(player);
        if (room->askForCard(ask_who, ".|black", "@thhuajiuse", data, objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (triggerEvent == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Nullification")) {
                if (room->askForCard(player, "Nullification", "@thhuaji:::nullification")) return false;
                room->setPlayerFlag(player, "thhuaji_cancel");
            } else {
                QString name = use.card->getClassName();
                if (name.endsWith("Slash"))
                    name = "Slash";
                QString str = use.card->objectName();
                if (str.endsWith("_slash"))
                    str = "slash";
                if (room->askForCard(player, name, "@thhuaji:::" + str)) return false;
                use.nullified_list << "_ALL_TARGETS";
                data = QVariant::fromValue(use);
            }
        } else {
            QString name, str;
            if (triggerEvent == CardResponded) {
                CardResponseStruct resp = data.value<CardResponseStruct>();
                name = resp.m_card->getClassName();
                str = resp.m_card->objectName();
            }
            if (name.endsWith("Slash"))
                name = "Slash";
            if (str.endsWith("_slash"))
                str = "slash";
            if (room->askForCard(player, name, "@thhuaji:::" + str)) return false;
            room->setPlayerFlag(player, "thhuaji_cancel");
        }

        return false;
    }
};

class ThFeizhan: public MaxCardsSkill {
public:
    ThFeizhan(): MaxCardsSkill("thfeizhan$") {
    }

    virtual int getExtra(const Player *target) const{
        if (target->hasLordSkill(objectName())) {
            int n = 0;
            foreach (const Player *p, target->getAliveSiblings())
                if (p->getKingdom() == "hana")
                    ++n;
            return n;
        } else {
            return 0;
        }
    }
};

ThJiewuCard::ThJiewuCard() {
}

bool ThJiewuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    return targets.isEmpty() && !to_select->isNude() && Self->inMyAttackRange(to_select);
}

void ThJiewuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const {
    ServerPlayer *target = targets.first();
    int card_id = room->askForCardChosen(source, target, "he", "thjiewu");
    room->obtainCard(source, card_id, false);
    Slash *slash = new Slash(NoSuit, 0);
    slash->setSkillName("_thjiewu");
    if (target->canSlash(source, slash, false)) {
        source->addQinggangTag(slash);
        CardUseStruct use;
        use.card = slash;
        use.from = target;
        use.to << source;
        room->useCard(use, false);
    } else
        delete slash;
}

class ThJiewu: public ZeroCardViewAsSkill {
public:
    ThJiewu(): ZeroCardViewAsSkill("thjiewu") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return !player->hasUsed("ThJiewuCard");
    }

    virtual const Card *viewAs() const {
        return new ThJiewuCard;
    }
};

class ThGenxing: public TriggerSkill {
public:
    ThGenxing(): TriggerSkill("thgenxing") {
        events << EventPhaseStart;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        return TriggerSkill::triggerable(player)
            && player->getHp() == 1
            && player->getPhase() == Player::Start
            && player->getMark("@genxing") <= 0;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        LogMessage log;
        log.type = "#ThGenxing";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);

        room->setPlayerMark(player, "@genxing", 1);
        QStringList choices;
        if (player->isWounded())
            choices << "recover";
        choices << "draw";

        QString choice = room->askForChoice(player, objectName(), choices.join("+"));
        if (choice == "recover") {
            RecoverStruct recover;
            recover.who = player;
            room->recover(player, recover);
        } else
            player->drawCards(2);

        room->changeMaxHpForAwakenSkill(player);
        room->acquireSkill(player, "thmopao");
        return false;
    }
};

class ThMopao: public TriggerSkill {
public:
    ThMopao(): TriggerSkill("thmopao") {
        events << CardResponded;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardResponseStruct resp = data.value<CardResponseStruct>();
        if (resp.m_card->isKindOf("Jink"))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "@thmopao", true, true);
        if (target) {
            room->broadcastSkillInvoke(objectName());
            player->tag["ThMopaoTarget"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = player->tag["ThMopaoTarget"].value<ServerPlayer *>();
        player->tag.remove("ThMopaoTarget");
        if (target) {
            target->drawCards(1);
            room->damage(DamageStruct(objectName(), player, target, 1, DamageStruct::Fire));
        }

        return false;
    }
};

class ThBian: public TriggerSkill {
public:
    ThBian(): TriggerSkill("thbian") {
        events << Dying;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.who->getHp() > 0 || dying.who->isDead())
            return QStringList();

        if (player->hasFlag("ThBianUsed") || player->getHandcardNum() < 2)
            return QStringList();

        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        const Card *dummy = room->askForExchange(player, objectName(), 2, 2, false, "@thbian", true);
        if (dummy && dummy->subcardsLength() > 0) {
            LogMessage log;
            log.type = "$DiscardCardWithSkill";
            log.from = player;
            log.card_str = IntList2StringList(dummy->getSubcards()).join("+");
            log.arg = objectName();
            room->sendLog(log);
            CardMoveReason reason(CardMoveReason::S_REASON_THROW, player->objectName());
            room->moveCardTo(dummy, NULL, Player::DiscardPile, reason, true);
            delete dummy;
            room->notifySkillInvoked(player, objectName());
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        DyingStruct dying = data.value<DyingStruct>();
        room->setPlayerFlag(player, "ThBianUsed");
        room->loseHp(dying.who);

        return false;
    }
};

class ThBianClear: public TriggerSkill {
public:
    ThBianClear(): TriggerSkill("#thbian") {
        events << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer* &) const {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::NotActive) {
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasFlag("ThBianUsed"))
                    room->setPlayerFlag(p, "-ThBianUsed");
            }
        }
        return QStringList();
    }
};

class ThGuihang:public TriggerSkill{
public:
    ThGuihang():TriggerSkill("thguihang"){
        events << Dying;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player)) return QStringList();
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.who->getHp() > 0 || dying.who->isDead())
            return QStringList();
        if (dying.who->isKongcheng())
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

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        DyingStruct dying = data.value<DyingStruct>();
        int card_id;
        if (player == dying.who)
            card_id = room->askForCardShow(player, player, "thguihang")->getId();
        else
            card_id = room->askForCardChosen(player, dying.who, "h", objectName());

        room->showCard(dying.who, card_id);
        const Card *card = Sanguosha->getCard(card_id);
        if (card->isRed()) {
            room->throwCard(card_id, dying.who);
            RecoverStruct recover;
            recover.who = player;
            room->recover(dying.who, recover);
        }

        return false;
    }
};

ThWujianCard::ThWujianCard() {
}

bool ThWujianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || to_select == Self)
        return false;

    int rangefix = 0;
    if (Self->getWeapon() && Self->getWeapon()->getId() == subcards.first()) {
        const Weapon *card = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        rangefix += card->getRange() - Self->getAttackRange(false);
    }

    if (Self->getOffensiveHorse() && Self->getOffensiveHorse()->getId() == subcards.first())
        rangefix += 1;

    return Self->inMyAttackRange(to_select, rangefix);
}

void ThWujianCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    room->addPlayerMark(targets.first(), "@wujian");
    source->tag["ThWujianUsed"] = true;
}

class ThWujian: public OneCardViewAsSkill {
public:
    ThWujian(): OneCardViewAsSkill("thwujian") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ThWujianCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        ThWujianCard *card = new ThWujianCard;
        card->addSubcard(originalCard->getId());
        return card;
    }
};

class ThWujianClear: public TriggerSkill {
public:
    ThWujianClear(): TriggerSkill("#thwujian") {
        events << EventPhaseStart << Death;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (triggerEvent == EventPhaseStart || triggerEvent == Death) {
            if (triggerEvent == Death) {
                DeathStruct death = data.value<DeathStruct>();
                if (death.who != player)
                    return QStringList();
            }
            if (!player->tag.value("ThWujianUsed", false).toBool())
                return QStringList();
            bool invoke = false;
            if ((triggerEvent == EventPhaseStart && player->getPhase() == Player::RoundStart) || triggerEvent == Death)
                invoke = true;
            if (!invoke)
                return QStringList();
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach (ServerPlayer *player, players)
                room->setPlayerMark(player, "@wujian", 0);
            player->tag.remove("ThWujianUsed");
        }

        return QStringList();
    }
};

class ThWujianDistanceSkill: public DistanceSkill{
public:
    ThWujianDistanceSkill(): DistanceSkill("#thwujian-distance") {
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        return from->getMark("@wujian");
    }
};

class ThXuelan: public TriggerSkill {
public:
    ThXuelan(): TriggerSkill("thxuelan") {
        events << CardEffected;
    }

    virtual TriggerList triggerable(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const {
        TriggerList skill_list;
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if (effect.card->isKindOf("Peach") && !effect.nullified) {
            foreach (ServerPlayer *owner, room->findPlayersBySkillName(objectName())) {
                if (owner->canDiscard(owner, "he"))
                    skill_list.insert(owner, QStringList(objectName()));
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const {
        if (room->askForCard(ask_who, ".|red", "@thxuelan", data, objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if (player->getMaxHp() <= player->getGeneralMaxHp()) {
            room->setPlayerProperty(player, "maxhp", player->getMaxHp() + 1);

            LogMessage log;
            log.type = "#GainMaxHp";
            log.from = player;
            log.arg = QString::number(1);
            room->sendLog(log);

            LogMessage log2;
            log2.type = "#GetHp";
            log2.from = player;
            log2.arg = QString::number(player->getHp());
            log2.arg2 = QString::number(player->getMaxHp());
            room->sendLog(log2);
        }

        effect.nullified = true;

        data = QVariant::fromValue(effect);

        return false;
    }
};

class ThXinwang: public TriggerSkill {
public:
    ThXinwang(): TriggerSkill("thxinwang") {
        events << CardsMoveOneTime << CardUsed << CardResponded;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player) || player == room->getCurrent())
            return QStringList();
        if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from != player) return QStringList();;

            if (triggerEvent == CardsMoveOneTime) {
                CardMoveReason reason = move.reason;
                if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
                    const Card *card;
                    QStringList list;
                    int i = 0;
                    foreach (int card_id, move.card_ids) {
                        card = Sanguosha->getCard(card_id);
                        if (card->getSuit() == Card::Heart && (move.from_places[i] == Player::PlaceHand
                                                               || move.from_places[i] == Player::PlaceEquip))
                            list << objectName();
                        i ++;
                    }
                    return list;
                }
            }
        } else {
            const Card *card = NULL;
            if (triggerEvent == CardUsed) {
                CardUseStruct use = data.value<CardUseStruct>();
                card = use.card;
            } else if (triggerEvent == CardResponded) {
                CardResponseStruct resp = data.value<CardResponseStruct>();
                card = resp.m_card;
            }
            if (card && card->getSuit() == Card::Heart) {
                return QStringList(objectName());
            }
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }

        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(player))
            if (p->isWounded())
                targets << p;
        ServerPlayer *target = NULL;
        if (!targets.isEmpty())
            target = room->askForPlayerChosen(player, targets, objectName(), "@thxinwang", true);
        if (target)
            room->recover(target, RecoverStruct(player));
        else
            player->drawCards(1);
        return false;
    }
};

class ThJuedu: public TriggerSkill {
public:
    ThJuedu():TriggerSkill("thjuedu") {
        events << Death << GameStart;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (triggerEvent == GameStart) {
            if (player == NULL) {
                const TriggerSkill *benghuai = Sanguosha->getTriggerSkill("ikbenghuai");
                if (benghuai)
                    room->getThread()->addTriggerSkill(benghuai);
            }
            return QStringList();
        }
        if (player && player->hasSkill(objectName())) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return QStringList();

            ServerPlayer *killer = death.damage ? death.damage->from : NULL;

            if (killer)
                if (killer != player)
                    return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *) const {
        DeathStruct death = data.value<DeathStruct>();
        ServerPlayer *killer = death.damage->from;
        room->addPlayerMark(killer, "@juedu");
        room->acquireSkill(killer, "ikbenghuai");

        return false;
    }
};

class ThTingwu: public TriggerSkill {
public:
    ThTingwu(): TriggerSkill("thtingwu") {
        events << PreDamageDone << DamageComplete;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
        DamageStruct damage = data.value<DamageStruct>();
        if (triggerEvent == PreDamageDone) {
            ServerPlayer *yijiu = damage.from;
            if (!player->isChained() && damage.nature == DamageStruct::Thunder && yijiu)
                yijiu->tag["ThTingwuTarget"] = QVariant::fromValue(room->findPlayer(player->getNextAlive()->objectName()));
            else if (yijiu)
                yijiu->tag.remove("ThTingwuTarget");
        } else if (triggerEvent == DamageComplete && TriggerSkill::triggerable(damage.from)) {
            ServerPlayer *target = damage.from->tag["ThTingwuTarget"].value<ServerPlayer *>();
            if (target && target->isAlive()) {
                ask_who = damage.from;
                return QStringList(objectName());
            }
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const {
        ServerPlayer *target = ask_who->tag["ThTingwuTarget"].value<ServerPlayer *>();
        if (target && ask_who->askForSkillInvoke(objectName(), ask_who->tag["ThTingwuTarget"])) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        ask_who->tag.remove("ThTingwuTarget");
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const {
        ServerPlayer *target = ask_who->tag["ThTingwuTarget"].value<ServerPlayer *>();
        ask_who->tag.remove("ThTingwuTarget");
        if (target) {
            JudgeStruct judge;
            judge.pattern = ".|heart";
            judge.good = false;
            judge.reason = objectName();
            judge.who = ask_who;
            room->judge(judge);

            if (judge.isGood())
                room->damage(DamageStruct(objectName(), ask_who, target, 1, DamageStruct::Thunder));
        }

        return false;
    }
};

class ThYuchang: public FilterSkill {
public:
    ThYuchang(): FilterSkill("thyuchang") {
    }

    virtual bool viewFilter(const Card* to_select) const {
        return to_select->isKindOf("Slash") && to_select->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        ThunderSlash *slash = new ThunderSlash(originalCard->getSuit(), originalCard->getNumber());
        slash->setSkillName(objectName());
        WrappedCard *card = Sanguosha->getWrappedCard(originalCard->getId());
        card->takeOver(slash);
        return card;
    }
};

class ThYuchangTargetMod: public TargetModSkill {
public:
    ThYuchangTargetMod(): TargetModSkill("#thyuchang-target") {
    }

    virtual int getDistanceLimit(const Player *from, const Card *card) const {
        if (from->hasSkill("thyuchang") && card->isKindOf("ThunderSlash"))
            return 1000;
        else
            return 0;
    }
};

ThXihuaCard::ThXihuaCard() {
    will_throw = false;
    handling_method = MethodNone;
}

bool ThXihuaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || to_select == Self)
        return false;
    Duel *duel = new Duel(NoSuit, 0);
    duel->setSkillName("_thxihua");
    duel->deleteLater();
    Slash *slash = new Slash(NoSuit, 0);
    slash->setSkillName("_thxihua");
    slash->deleteLater();
    if (Self->isProhibited(to_select, duel) && !Self->canSlash(to_select, slash, false))
        return false;
    if (Self->isCardLimited(duel, MethodUse) && Self->isCardLimited(slash, MethodUse))
        return false;
    return true;
}

void ThXihuaCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    source->addToPile("xihuapile", this, false);
    ServerPlayer *target = targets[0];
    QStringList choices;
    Slash *slash = new Slash(NoSuit, 0);
    slash->setSkillName("_thxihua");
    slash->deleteLater();
    Duel *duel = new Duel(NoSuit, 0);
    duel->setSkillName("_thxihua");
    duel->deleteLater();
    if (slash->isAvailable(source) && source->canSlash(target, slash, false))
        choices << "slash";

    if (duel->isAvailable(source) && !source->isProhibited(target, duel))
        choices << "duel";

    if (!choices.isEmpty()) {
        QString choice = room->askForChoice(source, "thxihua", choices.join("+"));
        if (choice == "slash") {
            CardUseStruct use;
            use.from = source;
            use.to << target;
            use.card = slash;
            room->useCard(use, false);
        } else if (choice == "duel") {
            CardUseStruct use;
            use.from = source;
            use.to << target;
            use.card = duel;
            room->useCard(use, false);
        }
    }

    source->clearOnePrivatePile("xihuapile");
}

class ThXihuaViewAsSkill: public OneCardViewAsSkill {
public:
    ThXihuaViewAsSkill(): OneCardViewAsSkill("thxihua") {
        filter_pattern = ".|.|.|hand";
        response_pattern = "@@thxihua";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        ThXihuaCard *card = new ThXihuaCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class ThXihua: public TriggerSkill {
public:
    ThXihua(): TriggerSkill("thxihua") {
        events << Predamage << EventPhaseStart;
        view_as_skill = new ThXihuaViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (triggerEvent == EventPhaseStart && TriggerSkill::triggerable(player)
            && player->getPhase() == Player::Start && !player->isKongcheng())
            return QStringList(objectName());
        else if (triggerEvent == Predamage) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->getSkillName() == objectName())
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (triggerEvent == EventPhaseStart)
            room->askForUseCard(player, "@@thxihua", "@thxihua", -1, Card::MethodNone);
        else
            return true;
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *) const {
        DamageStruct damage = data.value<DamageStruct>();
        int id = -1;
        ServerPlayer *owner, *victim;
        if (damage.from && !damage.from->getPile("xihuapile").isEmpty()) {
            id = damage.from->getPile("xihuapile").first();
            owner = damage.from;
            victim = damage.to;
        } else if (damage.to && !damage.to->getPile("xihuapile").isEmpty()) {
            id = damage.to->getPile("xihuapile").first();
            owner = damage.to;
            victim = damage.from;
        }

        if (id == -1)
            return true;

        room->showCard(owner, id);
        if (Sanguosha->getCard(id)->isKindOf("Slash")) {
            if (owner->canDiscard(victim, "h")) {
                int card_id = room->askForCardChosen(owner, victim, "h", objectName(), false, Card::MethodDiscard);
                room->throwCard(card_id, victim, owner);
            }
        } else
            return true;

        return false;
    }
};

ThMimengDialog *ThMimengDialog::getInstance(const QString &object, bool left, bool right,
                                      bool play_only, bool slash_combined, bool delayed_tricks) {
    static ThMimengDialog *instance = NULL;
    if (instance == NULL || instance->objectName() != object)
        instance = new ThMimengDialog(object, left, right, play_only, slash_combined, delayed_tricks);

    return instance;
}

ThMimengDialog::ThMimengDialog(const QString &object, bool left, bool right, bool play_only,
                               bool slash_combined, bool delayed_tricks)
    : object_name(object), play_only(play_only),
      slash_combined(slash_combined), delayed_tricks(delayed_tricks)
{
    setObjectName(object_name);
    setWindowTitle(Sanguosha->translate(object));
    group = new QButtonGroup(this);

    QHBoxLayout *layout = new QHBoxLayout;
    if (left) layout->addWidget(createLeft());
    if (right) layout->addWidget(createRight());
    setLayout(layout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(selectCard(QAbstractButton *)));
}

bool ThMimengDialog::isButtonEnabled(const QString &button_name) const{
    const Card *card = map[button_name];
    QStringList ban_list;
    if (object_name == "thmimeng")
        ban_list << "ExNihilo" << "AmazingGrace" << "Snatch" << "GodSalvation" << "ArcheryAttack"
                 << "Drowning" << "BurningCamps" << "LureTiger" << "KnownBoth";
    if (object_name == "ikmice")
        ban_list << "Drowning";
    if (object_name == "ikxieke" && Self->aliveCount() == 2)
        ban_list << "Jink" << "Analeptic" << "Peach";
    return !ban_list.contains(card->getClassName()) && !Self->isCardLimited(card, Card::MethodUse, true) && card->isAvailable(Self);
}

void ThMimengDialog::popup() {
    if (play_only && Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_PLAY) {
        emit onButtonClick();
        return;
    }

    bool has_enabled_button = false;
    foreach (QAbstractButton *button, group->buttons()) {
        bool enabled = isButtonEnabled(button->objectName());
        if (enabled)
            has_enabled_button = true;
        button->setEnabled(enabled);
    }
    if (!has_enabled_button) {
        emit onButtonClick();
        return;
    }

    Self->tag.remove(object_name);
    exec();
}

void ThMimengDialog::selectCard(QAbstractButton *button){
    const Card *card = map.value(button->objectName());
    Self->tag[object_name] = QVariant::fromValue(card);
    if (button->objectName().contains("slash")) {
        if (objectName() == "ikguihuo")
            Self->tag["IkGuihuoSlash"] = button->objectName();
    }
    emit onButtonClick();
    accept();
}

QGroupBox *ThMimengDialog::createLeft() {
    QGroupBox *box = new QGroupBox;
    box->setTitle(Sanguosha->translate("basic"));

    QVBoxLayout *layout = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach (const Card *card, cards) {
        if (card->getTypeId() == Card::TypeBasic && !map.contains(card->objectName())
            && !ServerInfo.Extensions.contains("!" + card->getPackage())
            && !(slash_combined && map.contains("slash") && card->objectName().contains("slash"))) {
            Card *c = Sanguosha->cloneCard(card->objectName());
            c->setParent(this);
            layout->addWidget(createButton(c));

            if (!slash_combined && objectName() == "ikguihuo" && card->objectName() == "slash"
                && !ServerInfo.Extensions.contains("!maneuvering")) {
                Card *c2 = Sanguosha->cloneCard(card->objectName());
                c2->setParent(this);
                layout->addWidget(createButton(c2));
            }
        }
    }

    layout->addStretch();
    box->setLayout(layout);
    return box;
}

QGroupBox *ThMimengDialog::createRight() {
    QGroupBox *box = new QGroupBox(Sanguosha->translate("trick"));
    QHBoxLayout *layout = new QHBoxLayout;

    QGroupBox *box1 = new QGroupBox(Sanguosha->translate("single_target_trick"));
    QVBoxLayout *layout1 = new QVBoxLayout;

    QGroupBox *box2 = new QGroupBox(Sanguosha->translate("multiple_target_trick"));
    QVBoxLayout *layout2 = new QVBoxLayout;

    QGroupBox *box3 = new QGroupBox(Sanguosha->translate("delayed_trick"));
    QVBoxLayout *layout3 = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if (card->getTypeId() == Card::TypeTrick && (delayed_tricks || card->isNDTrick())
            && !map.contains(card->objectName())
            && !ServerInfo.Extensions.contains("!" + card->getPackage())) {
            Card *c = Sanguosha->cloneCard(card->objectName());
            c->setSkillName(object_name);
            c->setParent(this);

            QVBoxLayout *layout;
            if (c->isKindOf("DelayedTrick"))
                layout = layout3;
            else if (c->isKindOf("SingleTargetTrick"))
                layout = layout1;
            else
                layout = layout2;
            layout->addWidget(createButton(c));
        }
    }

    box->setLayout(layout);
    box1->setLayout(layout1);
    box2->setLayout(layout2);
    box3->setLayout(layout3);

    layout1->addStretch();
    layout2->addStretch();
    layout3->addStretch();

    layout->addWidget(box1);
    layout->addWidget(box2);
    if (delayed_tricks)
        layout->addWidget(box3);
    return box;
}

QAbstractButton *ThMimengDialog::createButton(const Card *card){
    if (card->objectName() == "slash" && map.contains(card->objectName()) && !map.contains("normal_slash")) {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate("normal_slash"));
        button->setObjectName("normal_slash");
        button->setToolTip(card->getDescription());

        map.insert("normal_slash", card);
        group->addButton(button);

        return button;
    } else {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(card->objectName()));
        button->setObjectName(card->objectName());
        button->setToolTip(card->getDescription());

        map.insert(card->objectName(), card);
        group->addButton(button);

        return button;
    }
}

ThMimengCard::ThMimengCard() {
    mute = true;
    will_throw = false;
}

bool ThMimengCard::targetFixed() const {
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty()) {
            const Card *oc = Sanguosha->getCard(subcards.first());
            card = Sanguosha->cloneCard(user_string.split("+").first(), oc->getSuit(), oc->getNumber());
        }
        return card && card->targetFixed();
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    const Card *card = Self->tag.value("thmimeng").value<const Card *>();
    const Card *oc = Sanguosha->getCard(subcards.first());
    Card *new_card = Sanguosha->cloneCard(card->objectName(), oc->getSuit(), oc->getNumber());
    new_card->addSubcard(oc);
    new_card->setSkillName("thmimeng");
    return new_card && new_card->targetFixed();
}

bool ThMimengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty()) {
            const Card *oc = Sanguosha->getCard(subcards.first());
            card = Sanguosha->cloneCard(user_string.split("+").first(), oc->getSuit(), oc->getNumber());
        }
        return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return false;
    }

    const Card *card = Self->tag.value("thmimeng").value<const Card *>();
    const Card *oc = Sanguosha->getCard(subcards.first());
    Card *new_card = Sanguosha->cloneCard(card->objectName(), oc->getSuit(), oc->getNumber());
    new_card->addSubcard(oc);
    new_card->setSkillName("thmimeng");
    return new_card && new_card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, new_card, targets);
}

bool ThMimengCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty()) {
            const Card *oc = Sanguosha->getCard(subcards.first());
            card = Sanguosha->cloneCard(user_string.split("+").first(), oc->getSuit(), oc->getNumber());
        }
        return card && card->targetsFeasible(targets, Self);
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    const Card *card = Self->tag.value("thmimeng").value<const Card *>();
    const Card *oc = Sanguosha->getCard(subcards.first());
    Card *new_card = Sanguosha->cloneCard(card->objectName(), oc->getSuit(), oc->getNumber());
    new_card->addSubcard(oc);
    new_card->setSkillName("thmimeng");
    return new_card && new_card->targetsFeasible(targets, Self);
}

const Card *ThMimengCard::validate(CardUseStruct &card_use) const {
    ServerPlayer *thmimeng_general = card_use.from;

    Room *room = thmimeng_general->getRoom();
    QString to_use = user_string;

    if (user_string == "slash"
        && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        QStringList use_list;
        use_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            use_list << "thunder_slash" << "fire_slash";
        to_use = room->askForChoice(thmimeng_general, "thmimeng_skill_slash", use_list.join("+"));
    }

    const Card *card = Sanguosha->getCard(subcards.first());
    Card *use_card = Sanguosha->cloneCard(to_use, card->getSuit(), card->getNumber());
    use_card->setSkillName("thmimeng");
    use_card->addSubcard(subcards.first());
    use_card->deleteLater();

    return use_card;
}

const Card *ThMimengCard::validateInResponse(ServerPlayer *user) const{
    Room *room = user->getRoom();

    QString to_use;
    if (user_string == "peach+analeptic") {
        QStringList use_list;
        use_list << "peach";
        if (!Config.BanPackages.contains("maneuvering"))
            use_list << "analeptic";
        to_use = room->askForChoice(user, "thmimeng_skill_saveself", use_list.join("+"));
    } else if (user_string == "slash") {
        QStringList use_list;
        use_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            use_list << "thunder_slash" << "fire_slash";
        to_use = room->askForChoice(user, "thmimeng_skill_slash", use_list.join("+"));
    } else
        to_use = user_string;

    const Card *card = Sanguosha->getCard(subcards.first());
    Card *use_card = Sanguosha->cloneCard(to_use, card->getSuit(), card->getNumber());
    use_card->setSkillName("thmimeng");
    use_card->addSubcard(subcards.first());
    use_card->deleteLater();
    return use_card;
}

class ThMimeng: public OneCardViewAsSkill {
public:
    ThMimeng(): OneCardViewAsSkill("thmimeng") {
        filter_pattern = ".|.|.|hand";
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const {
        if (player->getHandcardNum() != 1 || pattern.startsWith(".") || pattern.startsWith("@")) return false;
        if (pattern == "peach" && player->hasFlag("Global_PreventPeach")) return false;
        for (int i = 0; i < pattern.length(); i++) {
            QChar ch = pattern[i];
            if (ch.isUpper() || ch.isDigit()) return false;
        }
        return true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return player->getHandcardNum() == 1;
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE
            || Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
            ThMimengCard *card = new ThMimengCard;
            card->setUserString(Sanguosha->currentRoomState()->getCurrentCardUsePattern());
            card->addSubcard(originalCard);
            return card;
        }

        const Card *c = Self->tag.value("thmimeng").value<const Card *>();
        if (c) {
            ThMimengCard *card = new ThMimengCard;
            card->setUserString(c->objectName());
            card->addSubcard(originalCard);
            return card;
        } else
            return NULL;
    }

    virtual QDialog *getDialog() const {
        return ThMimengDialog::getInstance("thmimeng");
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const {
        return player->getHandcardNum() == 1;
    }
};

class ThAnyun: public TriggerSkill {
public:
    ThAnyun(): TriggerSkill("thanyun") {
        events << EventPhaseStart;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const {
        if (player->getPhase() == Player::Draw && TriggerSkill::triggerable(player)) {
            return QStringList(objectName());
        } else if (player->getPhase() == Player::Finish && player->hasFlag("thanyun")) {
            return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (player->getPhase() == Player::Draw && player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName(), qrand() % 2 + 1);
            return true;
        } else if (player->getPhase() == Player::Finish) {
            room->broadcastSkillInvoke(objectName(), 3);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (player->getPhase() == Player::Draw) {
            room->setPlayerFlag(player, "thanyun");
            return true;
        } else {
            room->setPlayerFlag(player, "-thanyun");
            player->drawCards(2);
        }
        return false;
    }
};

ThQuanshanGiveCard::ThQuanshanGiveCard() {
    will_throw = false;
    handling_method = MethodNone;
}

bool ThQuanshanGiveCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    return targets.isEmpty() && to_select != Self && !to_select->hasFlag("thquanshan");
}

void ThQuanshanGiveCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    bool can_draw = true;
    CardType typeId = Sanguosha->getCard(subcards.first())->getTypeId();
    foreach (int id, subcards) {
        const Card *card = Sanguosha->getCard(id);
        if (card->getTypeId() != typeId) {
            can_draw = false;
            break;
        }
    }

    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), targets.first()->objectName(), "thquanshan", QString());
    room->obtainCard(targets.first(), this, reason);

    if (can_draw) {
        foreach (ServerPlayer *p, room->getAlivePlayers())
            if (p->hasFlag("thquanshan")) {
                p->drawCards(1, "thquanshan");
                break;
            }
    }
}

class ThQuanshanGive: public ViewAsSkill {
public:
    ThQuanshanGive(): ViewAsSkill("thquanshangive") {
        response_pattern = "@@thquanshangive!";
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const {
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const {
        if (cards.isEmpty())
            return NULL;

        Card *card = new ThQuanshanGiveCard;
        card->addSubcards(cards);
        return card;
    }
};

ThQuanshanCard::ThQuanshanCard() {
}

bool ThQuanshanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void ThQuanshanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const {
    ServerPlayer *target = targets.first();
    room->setPlayerFlag(source, "thquanshan");
    bool used = room->askForUseCard(target, "@@thquanshangive!", "@thquanshan", -1, MethodNone);
    room->setPlayerFlag(source, "-thquanshan");

    if (!used) {
        QList<ServerPlayer *>beggars = room->getOtherPlayers(target);
        beggars.removeOne(source);
        if (beggars.isEmpty()) return;

        qShuffle(beggars);

        ServerPlayer *beggar = beggars.at(0);

        QList<int> to_give = target->handCards().mid(0, 1);
        ThQuanshanGiveCard *quanshan_card = new ThQuanshanGiveCard;
        quanshan_card->addSubcards(to_give);
        QList<ServerPlayer *> targets;
        targets << beggar;
        quanshan_card->use(room, target, targets);
        delete quanshan_card;
    }
}

class ThQuanshan: public ZeroCardViewAsSkill {
public:
    ThQuanshan(): ZeroCardViewAsSkill("thquanshan") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ThQuanshanCard") && player->aliveCount() > 2;
    }

    virtual const Card *viewAs() const{
        return new ThQuanshanCard;
    }
};

class ThXiangang: public TriggerSkill {
public:
    ThXiangang(): TriggerSkill("thxiangang") {
        events << DamageInflicted;
        frequency = Frequent;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        JudgeStruct judge;
        judge.pattern = ".|club";
        judge.good = true;
        judge.reason = objectName();
        judge.who = player;
        room->judge(judge);

        if (judge.isGood()) {
            LogMessage log;
            log.type = "#thxiangang";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            return true;
        }

        return false;
    }
};

ThDuanzuiCard::ThDuanzuiCard() {
}

bool ThDuanzuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void ThDuanzuiCard::onEffect(const CardEffectStruct &effect) const {
    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "h", objectName());
    room->showCard(effect.to, card_id);
    const Card *card = Sanguosha->getCard(card_id);
    if (card->isKindOf("Slash")) {
        CardUseStruct use;
        use.from = effect.from;
        use.to << effect.to;
        Duel *use_card = new Duel(NoSuit, 0);
        use_card->setCancelable(false);
        use_card->setSkillName("_thduanzui");
        use_card->deleteLater();
        use.card = use_card;
        if (!effect.from->isProhibited(effect.to, use_card))
            room->useCard(use);
    } else if (card->isKindOf("Jink")) {
        CardUseStruct use;
        use.from = effect.from;
        use.to << effect.to;
        Slash *use_card = new Slash(NoSuit, 0);
        use_card->setSkillName("_thduanzui");
        use_card->deleteLater();
        use.card = use_card;
        if (effect.from->canSlash(effect.to, use_card, false))
            room->useCard(use, false);
    }
};

class ThDuanzui: public ZeroCardViewAsSkill {
public:
    ThDuanzui(): ZeroCardViewAsSkill("thduanzui") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ThDuanzuiCard");
    }

    virtual const Card *viewAs() const{
        return new ThDuanzuiCard;
    }
};

class ThYingdeng: public TriggerSkill {
public:
    ThYingdeng(): TriggerSkill("thyingdeng") {
        events << EventPhaseStart;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        if (player->getPhase() == Player::Start)
            if (!player->isKongcheng())
                return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        const Card *card = room->askForCard(player, "..", "@thyingdeng", data, objectName());
        if (card == NULL)
            return false;
        room->broadcastSkillInvoke(objectName());
        int n = (int)card->getSuit();
        n++;
        room->setPlayerMark(player, objectName(), n);
        return true;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        QString suitstr = Card::Suit2String(Card::Suit(player->getMark(objectName()) - 1));
        foreach(ServerPlayer *p, room->getOtherPlayers(player))
            room->setPlayerCardLimitation(p, "use,response", ".|" + suitstr, false);
        return false;
    }
};

class ThYingdengClear: public TriggerSkill {
public:
    ThYingdengClear(): TriggerSkill("#thyingdeng") {
        events << EventPhaseChanging << Death;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (player && player->getMark("thyingdeng") > 0) {
            if (triggerEvent == EventPhaseChanging) {
                PhaseChangeStruct change = data.value<PhaseChangeStruct>();
                if (change.to != Player::NotActive)
                    return QStringList();
            }
            if (triggerEvent == Death) {
                DeathStruct death = data.value<DeathStruct>();
                if (death.who != player) {
                    return QStringList();
                }
            }

            int n = player->getMark("thyingdeng") - 1;
            room->setPlayerMark(player, "thyingdeng", 0);
            QString suitstr = Card::Suit2String((Card::Suit)n);
            foreach(ServerPlayer *p, room->getOtherPlayers(player))
                room->removePlayerCardLimitation(p, "use,response", ".|" + suitstr + "$0");
        }
        return QStringList();
    }
};

ThZheyinCard::ThZheyinCard() {
    target_fixed = true;
}

void ThZheyinCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const {
    Room *room = source->getRoom();
    source->drawCards(1, "thzheyin");
    if (!source->isNude()) {
        const Card *card = NULL;
        card = room->askForCard(source, "..!", "@thzheyin", QVariant(), MethodNone);
        if (!card) {
            QList<const Card *> cards = source->getCards("he");
            card = cards.at(qrand() % cards.length());
        }
        source->addToPile("thzheyinpile", card, false);
    }
}

class ThZheyinViewAsSkill: public ZeroCardViewAsSkill {
public:
    ThZheyinViewAsSkill(): ZeroCardViewAsSkill("thzheyin") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ThZheyinCard");
    }

    virtual const Card *viewAs() const{
        ThZheyinCard *card = new ThZheyinCard;
        return card;
    }
};

class ThZheyin: public TriggerSkill {
public:
    ThZheyin(): TriggerSkill("thzheyin") {
        events << EventPhaseStart << TargetConfirming << CardEffected;
        view_as_skill = new ThZheyinViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (triggerEvent == EventPhaseStart) {
            if (player->getPile("thzheyinpile").isEmpty()) return QStringList();
            if (player->getPhase() == Player::RoundStart)
                player->clearOnePrivatePile("thzheyinpile");
        } else if (triggerEvent == TargetConfirming) {
            if (player && player->isAlive() && !player->getPile("thzheyinpile").isEmpty()) {
                CardUseStruct use = data.value<CardUseStruct>();
                if (use.to.contains(player)) {
                    Card::Suit suit = use.card->getSuit();
                    foreach (int id, player->getPile("thzheyinpile")) {
                        if (Sanguosha->getCard(id)->getSuit() == suit) {
                            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                                if (!player->pileOpen("thzheyinpile", p->objectName()))
                                    return QStringList(objectName());
                            }
                        }
                    }
                }
            }
        } else if (triggerEvent == CardEffected) {
            if (player && player->isAlive() && !player->getPile("thzheyinpile").isEmpty()) {
                CardEffectStruct effect = data.value<CardEffectStruct>();
                if (!effect.nullified) {
                    Card::Suit suit = effect.card->getSuit();
                    foreach (int id, player->getPile("thzheyinpile")) {
                        if (Sanguosha->getCard(id)->getSuit() == suit) {
                            bool invoke = true;
                            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                                if (!player->pileOpen("thzheyinpile", p->objectName())) {
                                    invoke = false;
                                    break;
                                }
                            }
                            if (invoke)
                                return QStringList(objectName());
                        }
                    }
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (triggerEvent == TargetConfirming) {
            if (player->askForSkillInvoke(objectName())) {
                room->broadcastSkillInvoke(objectName());
                return true;
            }
        } else if (triggerEvent == CardEffected) {
            room->sendCompulsoryTriggerLog(player, objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (triggerEvent == TargetConfirming) {
            foreach (ServerPlayer *p, room->getOtherPlayers(player))
                player->setPileOpen("thzheyinpile", p->objectName());
            foreach (int id, player->getPile("thzheyinpile"))
                room->showCard(player, id);
        } else if (triggerEvent == CardEffected) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            effect.nullified = true;
            data = QVariant::fromValue(effect);
        }
        return false;
    }
};

ThYachuiCard::ThYachuiCard() {
    will_throw = false;
    handling_method = MethodNone;
}

bool ThYachuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && subcardsLength() <= to_select->getLostHp();
}

void ThYachuiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), targets.first()->objectName(), "thyachui", QString());
    room->obtainCard(targets.first(), this, reason);
    source->drawCards(subcardsLength());
}

class ThYachuiViewAsSkill: public ViewAsSkill {
public:
    ThYachuiViewAsSkill(): ViewAsSkill("thyachui") {
        response_pattern = "@@thyachui";
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        return !to_select->isEquipped() && to_select->isRed();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.isEmpty())
            return NULL;
        ThYachuiCard *card = new ThYachuiCard;
        card->addSubcards(cards);
        return card;
    }
};

class ThYachui: public TriggerSkill {
public:
    ThYachui(): TriggerSkill("thyachui") {
        events << EventPhaseStart;
        view_as_skill = new ThYachuiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Draw && !player->isKongcheng())
            return true;
        return false;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (room->askForUseCard(player, "@@thyachui", "@thyachui", -1, Card::MethodNone))
            return true;
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer *) const {
        return true;
    }
};

class ThChunhen: public TriggerSkill{
public:
    ThChunhen():TriggerSkill("thchunhen") {
        events << BeforeCardsMove;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from == player || move.from == NULL)
            return QStringList();
        if (move.to_place == Player::DiscardPile
            && (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
            int i = 0;
            foreach (int card_id, move.card_ids) {
                if (Sanguosha->getCard(card_id)->getSuit() == Card::Diamond && room->getCardOwner(card_id) == move.from
                    && (move.from_places[i] == Player::PlaceHand || move.from_places[i] == Player::PlaceEquip))
                    return QStringList(objectName());
                i++;
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        player->drawCards(1);
        return false;
    }
};

class ThGuaitan: public TriggerSkill {
public:
    ThGuaitan():TriggerSkill("thguaitan") {
        events << EventPhaseStart << Damage;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer* &) const {
        if (triggerEvent == Damage && TriggerSkill::triggerable(player)) {
            return QStringList(objectName());
        } else if (triggerEvent == EventPhaseStart) {
            if (player->getPhase() == Player::RoundStart) {
                QStringList guaitanlist = player->tag.value("guaitan").toStringList();
                foreach(QString str, guaitanlist) {
                    LogMessage log;
                    log.type = "#ThGuaitanTrigger";
                    log.from = player;
                    log.arg = str;
                    log.arg2 = objectName();
                    room->sendLog(log);

                    room->setPlayerCardLimitation(player, "use,response", str, true);
                    player->tag["guaitan"] = QVariant::fromValue(QStringList());
                    room->setPlayerMark(player, objectName(), 1);
                }
            }
        }
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
        QString choice = room->askForChoice(player, objectName(), "BasicCard+EquipCard+TrickCard");
        DamageStruct damage = data.value<DamageStruct>();

        LogMessage log;
        log.type = "#ThGuaitan";
        log.from = player;
        log.to << damage.to;
        log.arg = choice;
        room->sendLog(log);

        room->setPlayerMark(damage.to, "@guaitan_" + choice.left(5).toLower(), 1);

        QStringList guaitanlist = damage.to->tag.value("guaitan").toStringList();
        choice = choice;
        if (!guaitanlist.contains(choice))
            guaitanlist << choice;

        damage.to->tag["guaitan"] = QVariant::fromValue(guaitanlist);

        return false;
    }
};

class ThGuaitanClear: public TriggerSkill {
public:
    ThGuaitanClear():TriggerSkill("#thguaitan") {
        events << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::NotActive && player->getMark("thguaitan") > 0) {
            room->setPlayerMark(player, "@guaitan_basic", 0);
            room->setPlayerMark(player, "@guaitan_equip", 0);
            room->setPlayerMark(player, "@guaitan_trick", 0);
            room->setPlayerMark(player, "thguaitan", 0);
        }

        return QStringList();
    }
};

class ThXiagong: public TargetModSkill {
public:
    ThXiagong(): TargetModSkill("thxiagong") {
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const {
        if (!from->getWeapon() && from->hasSkill(objectName()) && from->getAttackRange() < 2)
            return 2 - from->getAttackRange();
        else
            return 0;
    }
};

class ThHouzhi: public TriggerSkill {
public:
    ThHouzhi(): TriggerSkill("thhouzhi") {
        frequency = Compulsory;
        events << EventPhaseStart << DamageInflicted;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        if (triggerEvent == EventPhaseStart && player->getPhase() == Player::Finish && player->getMark("@jianren") > 0)
            return QStringList(objectName());
        if (triggerEvent == DamageInflicted && player->hasSkill("thliaoyu"))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(player, objectName());
        if (triggerEvent == EventPhaseStart) {
            int n = player->getMark("@jianren");
            player->loseAllMarks("@jianren");
            room->loseHp(player, n);
            return false;
        } else {
            DamageStruct damage = data.value<DamageStruct>();
            player->gainMark("@jianren", damage.damage);
            return true;
        }
    }
};

class ThLiaoyu: public TriggerSkill {
public:
    ThLiaoyu(): TriggerSkill("thliaoyu") {
        events << EventPhaseStart;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Start
            || player->getMark("@jianren") <= 0 || player->isNude())
            return QStringList();

        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (!room->askForCard(player, ".", "@thliaoyu", QVariant(), objectName()))
            return false;
        room->broadcastSkillInvoke(objectName());
        return true;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
        forever {
            QStringList chosen;
            if (player->getMark("@jianren") < 4) {
                QStringList choices;
                choices << "spade" << "heart" << "club" << "diamond";
                for (int i = 0; i < player->getMark("@jianren"); i++) {
                    QString choice = room->askForChoice(player, objectName(), choices.join("+"));
                    LogMessage log;
                    log.type = "#ChooseSuit";
                    log.from = player;
                    log.arg  = choice;
                    room->sendLog(log);
                    choices.removeOne(choice);
                    chosen << choice;
                }
            } else
                chosen << "spade" << "heart" << "club" << "diamond";

            JudgeStruct judge;
            judge.pattern = ".|" + chosen.join(",");
            judge.good = true;
            judge.reason = objectName();
            judge.who = player;
            room->judge(judge);

            if (judge.isGood())
                player->loseMark("@jianren");

            if (player->isNude() || player->getMark("@jianren") <= 0 || !cost(triggerEvent, room, player, data, ask_who))
                break;
        }

        return false;
    }
};

ThDujiaCard::ThDujiaCard() {
    target_fixed = true;
}

void ThDujiaCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->damage(DamageStruct("thdujia", source, source));
    source->drawCards(3, "thdujia");
}

class ThDujia: public OneCardViewAsSkill {
public:
    ThDujia(): OneCardViewAsSkill("thdujia") {
        filter_pattern = "BasicCard!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return !player->hasUsed("ThDujiaCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        ThDujiaCard *card = new ThDujiaCard;
        card->addSubcard(originalCard);
        return card;
    }
};

ThXianfaCard::ThXianfaCard() {
}

bool ThXianfaCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const{
    return targets.isEmpty();
}

void ThXianfaCard::onEffect(const CardEffectStruct &effect) const {
    Room *room = effect.from->getRoom();
    if (subcards.isEmpty())
        room->loseHp(effect.from);

    QString choice = room->askForChoice(effect.from, "thxianfa", "start+judge+draw+discard+finish");
    LogMessage log;
    log.type = "#ThXianfaChoose";
    log.from = effect.from;
    log.arg = choice;
    room->sendLog(log);

    effect.from->setFlags("thxianfa");
    effect.to->tag["ThXianfa"] = QVariant::fromValue(choice);
}

class ThXianfaViewAsSkill: public ViewAsSkill {
public:
    ThXianfaViewAsSkill(): ViewAsSkill("thxianfa") {

    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return !player->hasUsed("ThXianfaCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *) const {
        return selected.isEmpty();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const {
        if (cards.isEmpty())
            return new ThXianfaCard;
        else if (cards.length() == 1) {
            ThXianfaCard *card = new ThXianfaCard;
            card->addSubcards(cards);
            return card;
        } else
            return NULL;
    }
};

class ThXianfa: public TriggerSkill {
public:
    ThXianfa(): TriggerSkill("thxianfa") {
        events << EventPhaseChanging;
        view_as_skill = new ThXianfaViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (player->hasFlag("thxianfa") && change.from == Player::Discard)
            foreach (ServerPlayer *p, room->getAllPlayers())
                if (!p->tag.value("ThXianfa").isNull())
                    return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        player->setFlags("-thxianfa");
        ServerPlayer *target = NULL;
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (!p->tag.value("ThXianfa").isNull()) {
                target = p;
                break;
            }
        if (target && target->isAlive()) {
            Player::Phase phase = Player::PhaseNone;
            QString choice = target->tag.value("ThXianfa").toString();
            target->tag.remove("ThXianfa");

            LogMessage log;
            log.type = "#ThXianfaDo";
            log.from = target;
            log.arg = objectName();
            log.arg2 = choice;
            room->sendLog(log);

            if(choice == "start")
                phase = Player::Start;
            else if(choice == "judge")
                phase = Player::Judge;
            else if(choice == "draw")
                phase = Player::Draw;
            else if(choice == "discard")
                phase = Player::Discard;
            else if(choice == "finish")
                phase = Player::Finish;
            if (phase != Player::PhaseNone) {
                Player::Phase origin_phase = target->getPhase();
                target->setPhase(phase);
                room->broadcastProperty(target, "phase");
                RoomThread *thread = room->getThread();
                if (!thread->trigger(EventPhaseStart, room, target))
                    thread->trigger(EventPhaseProceeding, room, target);
                thread->trigger(EventPhaseEnd, room, target);

                target->setPhase(origin_phase);
                room->broadcastProperty(target, "phase");
            }
        }

        return false;
    }
};

class ThWendao: public TriggerSkill {
public:
    ThWendao(): TriggerSkill("thwendao") {
        events << EventPhaseStart;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        if (!TriggerSkill::triggerable(player) || player->getMark("@wendao") > 0)
            return false;
        if (player->getPhase() != Player::Start || !player->isKongcheng())
            return false;
        return true;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        room->addPlayerMark(player, "@wendao");
        LogMessage log;
        log.type = "#ThWendao";
        log.from = player;
        log.arg  = objectName();
        room->sendLog(log);
        room->broadcastSkillInvoke(objectName());
        room->getThread()->delay();

        QStringList choices;
        if (player->isWounded())
            choices << "recover";
        choices << "draw";

        QString choice = room->askForChoice(player, objectName(), choices.join("+"));

        if (choice == "recover") {
            RecoverStruct recover;
            recover.who = player;
            room->recover(player, recover);
        } else
            player->drawCards(2);

        room->changeMaxHpForAwakenSkill(player);
        room->acquireSkill(player, "ikmitu");
        return false;
    }
};

class ThLeishi: public TriggerSkill {
public:
    ThLeishi(): TriggerSkill("thleishi") {
        events << Dying << DamageComplete;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const {
        if (triggerEvent == Dying && TriggerSkill::triggerable(player)) {
            DyingStruct dying = data.value<DyingStruct>();
            DamageStruct *damage = dying.damage;
            if (dying.who->isDead() || dying.who->getHp() > 0
                || !damage || damage->from != player)
                return QStringList();
            return QStringList(objectName());
        } else if (triggerEvent == DamageComplete) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.from || damage.from->isDead())
                return QStringList();
            ServerPlayer *target = damage.from->tag["ThLeishiTarget"].value<ServerPlayer *>();
            if (!target || target->isDead())
                return QStringList();
            ask_who = damage.from;
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (triggerEvent == Dying) {
            DyingStruct dying = data.value<DyingStruct>();
            QList<ServerPlayer *> targets;
            int min = 998;
            foreach (ServerPlayer *p, room->getOtherPlayers(dying.who)) {
                int dis = dying.who->distanceTo(p);
                if (dis == -1)
                    continue;
                if (targets.isEmpty() || dis == min) {
                    targets << p;
                    min = dying.who->distanceTo(p);
                } else if (dis < min) {
                    targets.clear();
                    targets << p;
                    min = dying.who->distanceTo(p);
                }
            }
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@thleishi", true, true);
            if (target) {
                room->broadcastSkillInvoke(objectName());
                player->tag["ThLeishiTarget"] = QVariant::fromValue(target);
                return false;
            }
            player->tag.remove("ThLeishiTarget");
        } else
            return true;
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const {
        ServerPlayer *target = ask_who->tag["ThLeishiTarget"].value<ServerPlayer *>();
        ask_who->tag.remove("ThLeishiTarget");
        if (target)
            room->damage(DamageStruct(objectName(), ask_who, target, 1, DamageStruct::Thunder));
        return false;
    }
};

class ThShanling: public TriggerSkill {
public:
    ThShanling(): TriggerSkill("thshanling") {
        events << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const {
        if (TriggerSkill::triggerable(target) && target->getPhase() == Player::Start) {
            bool can_invoke = true;
            foreach (ServerPlayer *p, target->getRoom()->getAllPlayers())
                if (target->getHp() > qMax(0, p->getHp())) {
                    can_invoke = false;
                    break;
                }
            return can_invoke;
        }
        return false;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        QList<ServerPlayer *> targets;
        int min = 998;
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            int dis = player->distanceTo(p);
            if (dis == -1)
                continue;
            if (targets.isEmpty() || dis == min) {
                targets << p;
                min = player->distanceTo(p);
            } else if (dis < min) {
                targets.clear();
                targets << p;
                min = player->distanceTo(p);
            }
        }
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@thshanling", true, true);
        if (target) {
            room->broadcastSkillInvoke(objectName());
            player->tag["ThShanlingTarget"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = player->tag["ThShanlingTarget"].value<ServerPlayer *>();
        player->tag.remove("ThShanlingTarget");
        if (target)
            room->damage(DamageStruct(objectName(), player, target, 1, DamageStruct::Thunder));
        return false;
    }
};

class ThShijieViewAsSkill: public OneCardViewAsSkill {
public:
    ThShijieViewAsSkill(): OneCardViewAsSkill("thshijie") {
        expand_pile = "shijiepile";
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool viewFilter(const Card *to_select) const{
        return Self->getPile("shijiepile").contains(to_select->getEffectiveId());
    }

    virtual const Card *viewAs(const Card *o_card) const{
        Nullification *null = new Nullification(o_card->getSuit(), o_card->getNumber());
        null->setSkillName(objectName());
        null->addSubcard(o_card);
        return null;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "nullification" && !player->getPile("shijiepile").isEmpty();
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        return !player->getPile("shijiepile").isEmpty();
    }
};

class ThShijie: public TriggerSkill {
public:
    ThShijie(): TriggerSkill("thshijie") {
        events << HpRecover;
        view_as_skill = new ThShijieViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        RecoverStruct recover = data.value<RecoverStruct>();
        QStringList skills;
        if (!TriggerSkill::triggerable(player)) return skills;
        for (int i = 0; i < recover.recover; i++)
            skills << objectName();
        return skills;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        player->addToPile("shijiepile", room->drawCard(), true);
        return false;
    }
};

ThShengzhiCard::ThShengzhiCard() {
    target_fixed = true;
}

class ThShengzhiViewAsSkill: public OneCardViewAsSkill {
public:
    ThShengzhiViewAsSkill(): OneCardViewAsSkill("thshengzhi") {
        response_pattern = "@@thshengzhi";
        filter_pattern = ".|.|.|shijiepile#.|black|.|hand";
        expand_pile = "shijiepile";
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        ThShengzhiCard *card = new ThShengzhiCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class ThShengzhi: public TriggerSkill {
public:
    ThShengzhi(): TriggerSkill("thshengzhi") {
        events << EventPhaseStart;
        view_as_skill = new ThShengzhiViewAsSkill;
    }

    virtual TriggerList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const {
        TriggerList skill_list;
        if (player->getPhase() != Player::RoundStart)
            return skill_list;

        QList<Player::Phase> phases = player->getPhases();
        QList<Player::Phase> invoke_phases;

        foreach (Player::Phase phase, phases) {
            if (phase == Player::NotActive || phase == Player::RoundStart || phase == Player::PhaseNone)
                continue;
            if (!player->isSkipped(phase))
                invoke_phases << phase;
        }
        if (invoke_phases.isEmpty())
            return skill_list;
        foreach (ServerPlayer *owner, room->findPlayersBySkillName(objectName())) {
            if (owner == player)
                continue;
            if (owner->isKongcheng() && owner->getPile("shijiepile").isEmpty())
                continue;
            skill_list.insert(owner, QStringList(objectName()));
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &, ServerPlayer *ask_who) const {
        if (room->askForUseCard(ask_who, "@@thshengzhi", "@thshengzhi", -1, Card::MethodDiscard))
            return true;
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const {
        QList<Player::Phase> phases = player->getPhases();
        QStringList choices;
        foreach (Player::Phase phase, phases) {
            if (phase == Player::NotActive || phase == Player::RoundStart || phase == Player::PhaseNone)
                continue;
            if (!player->isSkipped(phase))
                choices << player->getPhaseString(phase);
        }
        if (choices.isEmpty()) return false;
        QString choice = room->askForChoice(ask_who, objectName(), choices.join("+"));

        QMap<QString, Player::Phase> phase_map;
        phase_map.insert("start", Player::Start);
        phase_map.insert("judge", Player::Judge);
        phase_map.insert("draw", Player::Draw);
        phase_map.insert("play", Player::Play);
        phase_map.insert("discard", Player::Discard);
        phase_map.insert("finish", Player::Finish);

        if (phase_map.value(choice, Player::PhaseNone) != Player::PhaseNone)
            player->skip(phase_map.value(choice));
        if (choice == "draw")
            room->loseHp(ask_who);
        else if (choice == "play") {
            RecoverStruct recover;
            recover.who = ask_who;
            room->recover(player, recover);
        }
        return false;
    }
};

class ThZhaoyu: public TriggerSkill {
public:
    ThZhaoyu(): TriggerSkill("thzhaoyu") {
        events << EventPhaseStart;
    }

    virtual TriggerList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const {
        TriggerList skill_list;
        if (player->getPhase() != Player::Start)
            return skill_list;
        foreach (ServerPlayer *owner, room->findPlayersBySkillName(objectName()))
            if (!owner->isNude())
                skill_list.insert(owner, QStringList(objectName()));
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const {
        const Card *card = room->askForCard(ask_who, "..", "@thzhaoyu", data, Card::MethodNone);
        if (card) {
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = ask_who;
            log.arg  = objectName();
            room->sendLog(log);
            room->notifySkillInvoked(ask_who, objectName());
            room->broadcastSkillInvoke(objectName());
            ask_who->tag["ThZhaoyuCard"] = QVariant::fromValue(card);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const {
        const Card *card = ask_who->tag["ThZhaoyuCard"].value<const Card *>();
        ask_who->tag.remove("ThZhaoyuCard");
        if (card) {
            CardMoveReason reason(CardMoveReason::S_REASON_PUT, ask_who->objectName(), "thzhaoyu", QString());
            room->moveCardTo(card, NULL, Player::DrawPile, reason, false);
            if ((!player->getJudgingArea().isEmpty() || player->getWeapon())
                && ask_who->askForSkillInvoke("thzhaoyu-draw", "draw")) {
                QList<int> card_ids;
                room->getThread()->trigger(FetchDrawPileCard, room, NULL);
                QList<int> &draw = room->getDrawPile();
                if (draw.isEmpty())
                    room->swapPile();
                card_ids << draw.takeLast();
                CardsMoveStruct move(card_ids,
                                     ask_who,
                                     Player::PlaceHand,
                                     CardMoveReason(CardMoveReason::S_REASON_DRAW, ask_who->objectName(), objectName(), QString()));
                room->moveCardsAtomic(move, false);
            }
        }
        return false;
    }
};

class ThWuwu: public TriggerSkill {
public:
    ThWuwu(): TriggerSkill("thwuwu") {
        frequency = Compulsory;
        events << EventPhaseStart;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Discard || player->isKongcheng())
            return QStringList();
        return QStringList(objectName());
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        room->broadcastSkillInvoke(objectName());
        room->sendCompulsoryTriggerLog(player, objectName());
        room->askForDiscard(player, objectName(), 1, 1, false, false);
        return false;
    }
};

class ThRudao: public TriggerSkill {
public:
    ThRudao(): TriggerSkill("thrudao") {
        events << EventPhaseStart;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        if (!TriggerSkill::triggerable(player) || player->getPhase() != Player::Start || !player->isKongcheng())
            return false;
        if (player->getMark("@rudao") > 0)
            return false;
        return true;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        room->addPlayerMark(player, "@rudao");
        LogMessage log;
        log.type = "#ThRudao";
        log.from = player;
        log.arg  = objectName();
        room->sendLog(log);
        room->broadcastSkillInvoke(objectName());
        room->notifySkillInvoked(player, objectName());

        player->drawCards(2);
        room->changeMaxHpForAwakenSkill(player, -3);
        room->detachSkillFromPlayer(player, "thwuwu");
        return false;
    }
};

ThLiuzhenCard::ThLiuzhenCard() {
    handling_method = MethodNone;
}

bool ThLiuzhenCard::targetFilter(const QList<const Player *> &, const Player *to_select, const Player *player) const{
    return !to_select->hasFlag("liuzhenold") && to_select != player;
}

void ThLiuzhenCard::onEffect(const CardEffectStruct &effect) const {
    Room *room = effect.from->getRoom();
    room->setPlayerFlag(effect.to, "liuzhennew");
}

class ThLiuzhenViewAsSkill: public ZeroCardViewAsSkill {
public:
    ThLiuzhenViewAsSkill(): ZeroCardViewAsSkill("thliuzhen") {
        response_pattern = "@@thliuzhen";
    }

    virtual const Card *viewAs() const {
        return new ThLiuzhenCard;
    }
};

class ThLiuzhen: public TriggerSkill{
public:
    ThLiuzhen(): TriggerSkill("thliuzhen") {
        events << TargetSpecified << BeforeCardsMove << SlashMissed;
        view_as_skill = new ThLiuzhenViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        if (triggerEvent == TargetSpecified && player->getPhase() == Player::Play) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Slash") && !player->hasUsed("ThLiuzhenCard")) {
                foreach (ServerPlayer *p, room->getOtherPlayers(player))
                    if (player->inMyAttackRange(p) && !use.to.contains(p))
                        return QStringList(objectName());
            }
        } else if (triggerEvent == SlashMissed) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (effect.slash->hasFlag("liuzhenslash")) {
                return QStringList(objectName());
            }
        } else if (triggerEvent == BeforeCardsMove) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile
            && move.reason.m_reason == CardMoveReason::S_REASON_USE) {
                if (player->tag["thliuzhen_user"].toBool()) {
                    const Card *liuzhen_card = move.reason.m_extraData.value<const Card *>();
                    if (liuzhen_card && liuzhen_card->hasFlag("thliuzhen"))
                        return QStringList(objectName());
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (triggerEvent == TargetSpecified) {
            CardUseStruct use = data.value<CardUseStruct>();
            foreach (ServerPlayer *tar, use.to)
                room->setPlayerFlag(tar, "liuzhenold");
            //tag for ai
            player->tag["thliuzhen_carduse"] = data;
            if (room->askForUseCard(player, "@@thliuzhen", "@thliuzhen", -1, Card::MethodNone)) {
                player->tag.remove("thliuzhen_carduse");
                room->setCardFlag(use.card, "thliuzhen");
                player->tag["thliuzhen_user"] = true;
            }

            foreach (ServerPlayer *tar, use.to)
                room->setPlayerFlag(tar, "-liuzhenold");
            return false;
        } else if (triggerEvent == SlashMissed) {
            return true;
        } else if (triggerEvent == BeforeCardsMove) {
            player->tag["thliuzhen_user"] = false;
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        if (triggerEvent == SlashMissed) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            LogMessage log;
            log.type = "#ThLiuzhenMiss";
            log.from = player;
            log.to << effect.to;
            log.arg  = objectName();
            room->sendLog(log);
            if (!player->canDiscard(player, "he") || !room->askForDiscard(player, objectName(), 1, 1, true, true))
                room->loseHp(player);
        } else if (triggerEvent == BeforeCardsMove) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            const Card *card = move.reason.m_extraData.value<const Card *>();
            if (card) {
                card->setFlags("-thliuzhen");
                if (card->hasFlag("drank")) {
                    room->setCardFlag(card, "-drank");
                    card->setTag("drank", 0);
                }
                CardUseStruct use;
                use.card = card;
                room->setCardFlag(use.card, "liuzhenslash");
                foreach (ServerPlayer *p, room->getAllPlayers())
                    if (p->hasFlag("liuzhennew")) {
                        room->setPlayerFlag(p, "-liuzhennew");
                        if (player->canSlash(p, false))
                            use.to << p;
                    }
                if (use.to.isEmpty()) return false;
                room->sortByActionOrder(use.to);
                use.from = player;
                room->useCard(use, false);
                move.removeCardIds(move.card_ids);
                data = QVariant::fromValue(move);
            }
        }
        return false;
    }
};

class ThTianchanViewAsSkill: public OneCardViewAsSkill {
public:
    ThTianchanViewAsSkill(): OneCardViewAsSkill("thtianchanv") {
        attached_lord_skill = true;
        filter_pattern = ".|spade";
    }

    virtual bool shouldBeVisible(const Player *player) const{
        return player->getKingdom() == "hana";
    }

    virtual bool isEnabledAtPlay(const Player *) const {
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const {
        if (pattern != "peach" || player->getKingdom() != "hana") return false;
        QString str = player->property("currentdying").toString();
        foreach (const Player *p, player->getAliveSiblings())
            if (p->objectName() == str && p->hasLordSkill("thtianchan"))
                return true;
        return false;
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        Peach *card = new Peach(originalCard->getSuit(), originalCard->getNumber());
        card->addSubcard(originalCard);
        card->setSkillName("thtianchan");
        return card;
    }
};

class ThTianchan: public TriggerSkill {
public:
    ThTianchan(): TriggerSkill("thtianchan$") {
        events << GameStart << EventAcquireSkill << EventLoseSkill << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!player) return QStringList();
        if ((triggerEvent == GameStart && player->isLord())
            || (triggerEvent == EventAcquireSkill && data.toString() == "thtianchan")) {
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
                if (!p->hasSkill("thtianchanv"))
                    room->attachSkillToPlayer(p, "thtianchanv");
            }
        } else if (triggerEvent == EventLoseSkill && data.toString() == "thtianchan") {
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
                if (p->hasSkill("thtianchanv"))
                    room->detachSkillFromPlayer(p, "thtianchanv", true);
            }
        }
        return QStringList();
    }
};

TouhouHanaPackage::TouhouHanaPackage()
    :Package("touhou-hana")
{
    General *hana001 = new General(this, "hana001$", "hana");
    hana001->addSkill(new ThHuaji);
    hana001->addSkill(new ThFeizhan);

    General *hana002 = new General(this, "hana002", "hana");
    hana002->addSkill(new ThJiewu);
    hana002->addSkill(new SlashNoDistanceLimitSkill("thjiewu"));
    related_skills.insertMulti("thjiewu", "#thjiewu-slash-ndl");
    hana002->addSkill(new ThGenxing);
    hana002->addRelateSkill("thmopao");

    General *hana003 = new General(this, "hana003", "hana", 3, false);
    hana003->addSkill(new ThBian);
    hana003->addSkill(new ThBianClear);
    related_skills.insertMulti("thbian", "#thbian");
    hana003->addSkill(new ThGuihang);
    hana003->addSkill(new ThWujian);
    hana003->addSkill(new ThWujianClear);
    related_skills.insertMulti("thwujian", "#thwujian");
    skills << new ThWujianDistanceSkill;

    General *hana004 = new General(this, "hana004", "hana", 3);
    hana004->addSkill(new ThXuelan);
    hana004->addSkill(new ThXinwang);
    hana004->addSkill(new ThJuedu);

    General *hana005 = new General(this, "hana005", "hana");
    hana005->addSkill(new ThTingwu);
    hana005->addSkill(new ThYuchang);
    hana005->addSkill(new ThYuchangTargetMod);
    related_skills.insertMulti("thyuchang", "#thyuchang-target");

    General *hana006 = new General(this, "hana006", "hana");
    hana006->addSkill(new ThXihua);
    hana006->addSkill(new SlashNoDistanceLimitSkill("thxihua"));
    related_skills.insertMulti("thxihua", "#thxihua-slash-ndl");

    General *hana007 = new General(this, "hana007", "hana", 3, false);
    hana007->addSkill(new ThMimeng);
    hana007->addSkill(new ThAnyun);

    General *hana008 = new General(this, "hana008", "hana", 3);
    hana008->addSkill(new ThQuanshan);
    hana008->addSkill(new ThXiangang);

    General *hana009 = new General(this, "hana009", "hana");
    hana009->addSkill(new ThDuanzui);
    hana009->addSkill(new SlashNoDistanceLimitSkill("thduanzui"));
    related_skills.insertMulti("thduanzui", "#thduanzui-slash-ndl");

    General *hana010 = new General(this, "hana010", "hana");
    hana010->addSkill(new ThYingdeng);
    hana010->addSkill(new ThYingdengClear);
    related_skills.insertMulti("thyingdeng", "#thyingdeng");
    hana010->addSkill(new ThZheyin);

    General *hana011 = new General(this, "hana011", "hana", 3);
    hana011->addSkill(new ThYachui);
    hana011->addSkill(new ThChunhen);

    General *hana012 = new General(this, "hana012", "hana");
    hana012->addSkill(new ThGuaitan);
    hana012->addSkill(new ThGuaitanClear);
    related_skills.insertMulti("thguaitan", "#thguaitan");
    hana012->addSkill(new ThXiagong);

    General *hana013 = new General(this, "hana013", "hana", 3);
    hana013->addSkill(new ThHouzhi);
    hana013->addSkill(new ThLiaoyu);
    hana013->addSkill(new ThDujia);

    General *hana014 = new General(this, "hana014", "hana", 4, false);
    hana014->addSkill(new ThXianfa);
    hana014->addSkill(new ThWendao);

    General *hana015 = new General(this, "hana015", "hana", 3);
    hana015->addSkill(new ThLeishi);
    hana015->addSkill(new ThShanling);

    General *hana016 = new General(this, "hana016", "hana", 3);
    hana016->addSkill(new ThShijie);
    hana016->addSkill(new ThShengzhi);

    General *hana017 = new General(this, "hana017", "hana", 7);
    hana017->addSkill(new ThZhaoyu);
    hana017->addSkill(new ThWuwu);
    hana017->addSkill(new ThRudao);

    General *hana018 = new General(this, "hana018$", "hana");
    hana018->addSkill(new ThLiuzhen);
    hana018->addSkill(new ThTianchan);

    addMetaObject<ThJiewuCard>();
    addMetaObject<ThWujianCard>();
    addMetaObject<ThXihuaCard>();
    addMetaObject<ThMimengCard>();
    addMetaObject<ThQuanshanGiveCard>();
    addMetaObject<ThQuanshanCard>();
    addMetaObject<ThDuanzuiCard>();
    addMetaObject<ThZheyinCard>();
    addMetaObject<ThYachuiCard>();
    addMetaObject<ThDujiaCard>();
    addMetaObject<ThXianfaCard>();
    addMetaObject<ThShengzhiCard>();
    addMetaObject<ThLiuzhenCard>();

    skills << new ThMopao << new ThQuanshanGive << new ThTianchanViewAsSkill;
}

ADD_PACKAGE(TouhouHana)
