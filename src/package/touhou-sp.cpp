#include "touhou-sp.h"
#include "general.h"
#include "skill.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "client.h"

class ThFanshi: public OneCardViewAsSkill {
public:
    ThFanshi(): OneCardViewAsSkill("thfanshi") {
        filter_pattern = ".|red|.|hand";
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getHandcardNum() >= player->getHp() && Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if (player->getHandcardNum() < player->getHp())
            return false;
        return (pattern == "slash" && player->getPhase() != Player::NotActive)
            || ((pattern == "jink" || pattern.contains("peach")) && player->getPhase() == Player::NotActive);
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *card = NULL;
        switch (Sanguosha->currentRoomState()->getCurrentCardUseReason()) {
        case CardUseStruct::CARD_USE_REASON_PLAY: {
                card = new Slash(originalCard->getSuit(), originalCard->getNumber());
                break;
                                                  }
        case CardUseStruct::CARD_USE_REASON_RESPONSE:
        case CardUseStruct::CARD_USE_REASON_RESPONSE_USE: {
                QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
                if (pattern == "jink")
                    card = new Jink(originalCard->getSuit(), originalCard->getNumber());
                else if (pattern == "peach" || pattern == "peach+analeptic")
                    card = new Peach(originalCard->getSuit(), originalCard->getNumber());
                else if (pattern == "slash")
                    card = new Slash(originalCard->getSuit(), originalCard->getNumber());
                else
                    return NULL;
                break;
                                                          }
        default: {
            break;
                 }
        }
        if (card != NULL){
            card->setSkillName(objectName());
            card->addSubcard(originalCard);
            return card;
        }
        return NULL;
    }
};

class ThNichang: public TriggerSkill {
public:
    ThNichang(): TriggerSkill("thnichang") {
        events << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        return TriggerSkill::triggerable(player)
            && player->getPhase() == Player::Start;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "@thnichang", true, true);
        if (target) {
            room->broadcastSkillInvoke(objectName());
            player->tag["ThNichangTarget"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = player->tag["ThNichangTarget"].value<ServerPlayer *>();
        player->tag.remove("ThNichangTarget");
        if (target) {
            target->setChained(!target->isChained());
            room->broadcastProperty(target, "chained");
            room->setEmotion(target, "effects/iron_chain");
            room->getThread()->trigger(ChainStateChanged, room, target);
        }

        return false;
    }
};

class ThGuanjia: public TriggerSkill {
public:
    ThGuanjia(): TriggerSkill("thguanjia") {
        events << TargetSpecified << CardFinished;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        CardUseStruct use = data.value<CardUseStruct>();
        if (triggerEvent == TargetSpecified)
            foreach (ServerPlayer *p, room->getOtherPlayers(player))
                room->addPlayerMark(p, "Armor_Nullified");
        else
            foreach (ServerPlayer *p, room->getOtherPlayers(player))
                room->removePlayerMark(p, "Armor_Nullified");

        return QStringList();
    }
};

class ThShenshi: public TriggerSkill {
public:
    ThShenshi(): TriggerSkill("thshenshi") {
        events << EventPhaseSkipped << EventPhaseChanging;
    }

    virtual TriggerList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        TriggerList skill_list;
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive) {
                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    if (p->hasFlag("thshenshiused"))
                        p->setFlags("-thshenshiused");
                }
            }
        } else if (player->isWounded())
            foreach (ServerPlayer *owner, room->findPlayersBySkillName(objectName())) {
                if (owner->hasFlag("thshenshiused") || !owner->canDiscard(owner, "he")) continue;
                skill_list.insert(owner, QStringList(objectName()));
            }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
        if (room->askForCard(ask_who, "..", "@shenshi:" + player->objectName(), data, objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const {
        room->recover(player, RecoverStruct(ask_who));
        ask_who->setFlags("thshenshiused");
        return false;
    }
};

class ThJiefan: public TriggerSkill {
public:
    ThJiefan(): TriggerSkill("thjiefan") {
        events << EventPhaseStart;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        return TriggerSkill::triggerable(player)
            && player->getPhase() == Player::Start;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        int id = room->drawCard();

        CardsMoveStruct move;
        move.card_ids << id;
        move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, player->objectName(), objectName(), QString());
        move.to_place = Player::PlaceTable;
        room->moveCardsAtomic(move, true);
        room->getThread()->delay();

        const Card *card = Sanguosha->getCard(id);
        if (card->isKindOf("BasicCard")) {
            ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
            room->obtainCard(target, card);
        } else {
            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, player->objectName(), objectName(), QString());
            room->throwCard(card, reason, NULL);
            return false;
        }

        if (player->askForSkillInvoke(objectName()))
            effect(NonTrigger, room, player, data, player);
        return false;
    }
};

class ThChuangshi: public TriggerSkill {
public:
    ThChuangshi(): TriggerSkill("thchuangshi") {
        events << CardEffected;
    }

    virtual TriggerList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        TriggerList skill_list;
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if (!effect.card->isKindOf("Dismantlement")
            && !effect.card->isKindOf("Collateral")
            && !effect.card->isKindOf("Duel"))
            return skill_list;

        foreach (ServerPlayer *owner, room->findPlayersBySkillName(objectName())) {
            if (effect.from == owner) continue;
            if (owner == player || owner->inMyAttackRange(player))
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
        CardEffectStruct effect = data.value<CardEffectStruct>();
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("_" + objectName());
        if (effect.from->canSlash(ask_who, slash, false))
            room->useCard(CardUseStruct(slash, effect.from, ask_who));
        else
            delete slash;
        return true;
    }
};

class ThGaotian: public TriggerSkill {
public:
    ThGaotian(): TriggerSkill("thgaotian") {
        events << CardAsked;
        frequency = Frequent;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        QString asked_pattern = data.toStringList().first();
        if (TriggerSkill::triggerable(player) && asked_pattern == "jink")
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
        int n = qMin(4, room->alivePlayerCount());
        QList<int> card_ids = room->getNCards(n, false);

        room->fillAG(card_ids, player);

        QList<int> reds, blacks;
        foreach (int id, card_ids)
            if (Sanguosha->getCard(id)->isRed())
                reds << id;
            else if (Sanguosha->getCard(id)->isBlack())
                blacks << id;
        QString pattern = "..";
        if (reds.isEmpty())
            pattern = ".|black";
        else if (blacks.isEmpty())
            pattern = ".|red";
        const Card *card = room->askForCard(player, pattern, "@gaotian-discard", data, objectName());
        if (card) {
            int card_id = -1;
            if (card->isBlack()) {
                room->clearAG(player);
                room->fillAG(card_ids, player, reds);
                card_id = room->askForAG(player, blacks, false, objectName());
            } else if (card->isRed()) {
                room->clearAG(player);
                room->fillAG(card_ids, player, blacks);
                card_id = room->askForAG(player, reds, false, objectName());
            }
            if (card_id != -1) {
                room->clearAG(player);
                room->fillAG(card_ids, player);
                QList<ServerPlayer *> p_list;
                p_list << player;
                room->takeAG(player, card_id, false, p_list);
                card_ids.removeOne(card_id);
                room->obtainCard(player, card_id);
            }
        }

        DummyCard *dummy = new DummyCard;
        QList<ServerPlayer *> p_list;
        p_list << player;
        while (!card_ids.isEmpty()) {
            int card_id = room->askForAG(player, card_ids, true, objectName());
            if (card_id != -1) {
                room->takeAG(NULL, card_id, false, p_list);
                card_ids.removeOne(card_id);
                dummy->addSubcard(card_id);
            } else
                break;
        }

        room->clearAG(player);
        if (!card_ids.isEmpty())
            room->askForGuanxing(player, card_ids, Room::GuanxingUpOnly);
        if (dummy->subcardsLength() > 0) {
            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, player->objectName(), objectName(), QString());
            room->throwCard(dummy, reason, NULL);
        }
        delete dummy;

        return false;
    }
};

class ThWanling: public TriggerSkill {
public:
    ThWanling(): TriggerSkill("thwanling") {
        events << BeforeCardsMove;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (!TriggerSkill::triggerable(player) || player == move.from || !move.from) return QStringList();
        if (move.from_places.contains(Player::PlaceTable) && move.to_place == Player::DiscardPile
            && move.reason.m_reason == CardMoveReason::S_REASON_USE) {
            const Card *card = move.reason.m_extraData.value<const Card *>();
            if (card && card->isRed() && (card->isKindOf("Slash") || card->isNDTrick()))
                return QStringList(objectName());
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

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (player->canDiscard(player, "he") && room->askForCard(player,
                                                                 "..",
                                                                 "@thwanling:" + move.from->objectName(),
                                                                 data,
                                                                 objectName())) {
            player->obtainCard(move.reason.m_extraData.value<const Card *>());
            move.removeCardIds(move.card_ids);
            data = QVariant::fromValue(move);
        } else
            room->drawCards((ServerPlayer *)move.from, 1, objectName());
        return false;
    }
};

class ThZuibu: public TriggerSkill {
public:
    ThZuibu(): TriggerSkill("thzuibu") {
        events << DamageInflicted;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &ask_who) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (!damage.from || damage.from->isDead())
            return QStringList();
        ask_who = damage.from;
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const{
        if (ask_who->askForSkillInvoke(objectName(), QVariant::fromValue(player))) {
            room->broadcastSkillInvoke(objectName());
            LogMessage log;
            log.type = "#InvokeOthersSkill";
            log.from = ask_who;
            log.to << player;
            log.arg = objectName();
            room->sendLog(log);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const {
        player->drawCards(1, objectName());
        DamageStruct damage = data.value<DamageStruct>();
        LogMessage log;
        log.type = "#ThZuibu";
        log.from = ask_who;
        log.to << player;
        log.arg = QString::number(damage.damage);
        log.arg2 = QString::number(--damage.damage);
        room->sendLog(log);

        data = QVariant::fromValue(damage);
        if (damage.damage < 1)
            return true;
        return false;
    }
};

#include "jsonutils.h"
class ThModao: public PhaseChangeSkill {
public:
    ThModao(): PhaseChangeSkill("thmodao") {
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        if (!PhaseChangeSkill::triggerable(player) || player->getPhase() != Player::Draw)
            return false;
        QStringList flags;
        flags << "h" << "e" << "j";
        foreach (ServerPlayer *p, player->getRoom()->getAllPlayers()) {
            if (player->inMyAttackRange(p)) {
                foreach (QString flag, flags) {
                    if (qAbs(p->getCards(flag).length() - player->getCards(flag).length()) <= player->getLostHp() + 1)
                        return true;
                }
            }
        }
        return false;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        QList<ServerPlayer *> targets;
        QStringList flags;
        flags << "h" << "e" << "j";
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (player->inMyAttackRange(p)) {
                foreach (QString flag, flags) {
                    if (qAbs(p->getCards(flag).length() - player->getCards(flag).length()) <= player->getLostHp() + 1) {
                        targets << p;
                        break;
                    }
                }
            }
        }
        if (targets.isEmpty()) return false;
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@thmodao", true, true);
        if (target) {
            room->broadcastSkillInvoke(objectName());
            player->tag["ThModaoTarget"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const {
        Room *room = player->getRoom();
        ServerPlayer *target = player->tag["ThModaoTarget"].value<ServerPlayer *>();
        player->tag.value("ThModaoTarget");
        if (target) {
            QStringList flags;
            flags << "h" << "e" << "j";
            QStringList choices;
            foreach (QString flag, flags) {
                if (qAbs(target->getCards(flag).length() - player->getCards(flag).length()) <= player->getLostHp() + 1)
                    choices << flag;
            }
            QString choice = room->askForChoice(player, objectName(), choices.join("+"));
            if (choice == "h") {
                foreach (ServerPlayer *p, room->getAlivePlayers()) {
                    if (p != player && p != target)
                        room->doNotify(p, QSanProtocol::S_COMMAND_EXCHANGE_KNOWN_CARDS,
                                       QSanProtocol::Utils::toJsonArray(player->objectName(), target->objectName()));
                }
                QList<CardsMoveStruct> exchangeMove;
                CardsMoveStruct move1(player->handCards(), target, Player::PlaceHand,
                                      CardMoveReason(CardMoveReason::S_REASON_SWAP,
                                                     player->objectName(),
                                                     target->objectName(),
                                                     objectName(),
                                                     QString()));
                CardsMoveStruct move2(target->handCards(), player, Player::PlaceHand,
                                      CardMoveReason(CardMoveReason::S_REASON_SWAP,
                                                     target->objectName(),
                                                     player->objectName(),
                                                     objectName(),
                                                     QString()));
                exchangeMove.push_back(move1);
                exchangeMove.push_back(move2);
                room->moveCardsAtomic(exchangeMove, false);
            } else if (choice == "e") {
                QList<CardsMoveStruct> exchangeMove;
                QList<int> ids1;
                foreach (const Card *c, player->getEquips())
                    ids1 << c->getEffectiveId();
                CardsMoveStruct move1(ids1, target, Player::PlaceEquip,
                                      CardMoveReason(CardMoveReason::S_REASON_SWAP,
                                                     player->objectName(),
                                                     target->objectName(),
                                                     objectName(),
                                                     QString()));
                QList<int> ids2;
                foreach (const Card *c, target->getEquips())
                    ids2 << c->getEffectiveId();
                CardsMoveStruct move2(ids2, player, Player::PlaceEquip,
                                      CardMoveReason(CardMoveReason::S_REASON_SWAP,
                                                     target->objectName(),
                                                     player->objectName(),
                                                     objectName(),
                                                     QString()));
                exchangeMove.push_back(move1);
                exchangeMove.push_back(move2);
                room->moveCardsAtomic(exchangeMove, false);
            } else if (choice == "j") {
                QList<CardsMoveStruct> exchangeMove;
                CardsMoveStruct move1(player->getJudgingAreaID(), target, Player::PlaceDelayedTrick,
                                      CardMoveReason(CardMoveReason::S_REASON_SWAP,
                                                     player->objectName(),
                                                     target->objectName(),
                                                     objectName(),
                                                     QString()));
                CardsMoveStruct move2(target->getJudgingAreaID(), player, Player::PlaceDelayedTrick,
                                      CardMoveReason(CardMoveReason::S_REASON_SWAP,
                                                     target->objectName(),
                                                     player->objectName(),
                                                     objectName(),
                                                     QString()));
                exchangeMove.push_back(move1);
                exchangeMove.push_back(move2);
                room->moveCardsAtomic(exchangeMove, false);
            }
            player->drawCards(1);
            target->drawCards(1);
            return true;
        }
        return false;
    }
};

class ThMengsheng: public TriggerSkill {
public:
    ThMengsheng(): TriggerSkill("thmengsheng") {
        events << Damaged;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from && damage.from->isAlive() && TriggerSkill::triggerable(player))
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
        DamageStruct damage = data.value<DamageStruct>();
        if (!damage.from->isKongcheng())
            room->obtainCard(player, room->askForCardChosen(player, damage.from, "h", objectName()), false);

        QStringList sources = damage.from->tag["ThMengsheng"].toStringList();
        sources << player->objectName();
        damage.from->tag["ThMengsheng"] = QVariant::fromValue(sources);
        player->tag["ThMengshengRecord"] = true;
        room->setPlayerMark(damage.from, "@mengsheng", 1);
        foreach (ServerPlayer *pl, room->getAllPlayers())
            room->filterCards(pl, pl->getCards("he"), true);
        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
        if (player == room->getCurrent() && player->getPhase() != Player::NotActive)
            room->setPlayerFlag(player, "thmengsheng");

        return false;
    }
};

class ThMengshengClear: public TriggerSkill {
public:
    ThMengshengClear(): TriggerSkill("#thmengsheng-clear") {
        events << EventPhaseChanging << Death;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!player || !player->tag["ThMengshengRecord"].toBool()) return QStringList();
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (player->hasFlag("thmengsheng") || change.to != Player::NotActive)
                return QStringList();
        }
        if (triggerEvent == Death && data.value<DeathStruct>().who != player)
            return QStringList();
        player->tag["ThMengshengRecord"] = false;
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (p->tag.value("ThMengsheng").toStringList().contains(player->objectName())) {
                QStringList sources = p->tag.value("ThMengsheng").toStringList();
                sources.removeAll(player->objectName());
                if (sources.isEmpty()) {
                    room->setPlayerMark(p, "@mengsheng", 0);
                    foreach (ServerPlayer *pl, room->getAllPlayers())
                        room->filterCards(pl, pl->getCards("he"), false);
                    Json::Value args;
                    args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
                    room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
                }
                p->tag["ThMengsheng"] = QVariant::fromValue(sources);
            }
        return QStringList();
    }
};

class ThMengshengInvalidity: public InvaliditySkill {
public:
    ThMengshengInvalidity(): InvaliditySkill("#thmengsheng-inv") {
    }

    virtual bool isSkillValid(const Player *player, const Skill *) const{
        return player->getMark("@mengsheng") == 0;
    }
};

class ThQixiang: public TriggerSkill {
public:
    ThQixiang(): TriggerSkill("thqixiang") {
        events << EventPhaseEnd;
    }

    virtual TriggerList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const {
        TriggerList skill_list;
        if (player->getPhase() == Player::Discard) {
            foreach (ServerPlayer *owner, room->findPlayersBySkillName(objectName())) {
                if (owner->canDiscard(owner, "h") && owner->inMyAttackRange(player))
                    skill_list.insert(owner, QStringList(objectName()));
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *ask_who) const {
        if (room->askForCard(ask_who, ".", "@thqixiang", data, objectName())) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *ask_who) const {
        QStringList choices;
        choices << "draw";
        if (!player->isKongcheng())
            choices << "discard";
        QString choice = room->askForChoice(ask_who, objectName(), choices.join("+"));
        if (choice == "discard")
            room->askForDiscard(player, objectName(), 1, 1);
        else
            player->drawCards(1);
        return false;
    }
};

class ThHuanlongViewAsSkill: public ViewAsSkill {
public:
    ThHuanlongViewAsSkill(): ViewAsSkill("thhuanlong") {
        response_or_use = true;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const {
        if (selected.length() >= 2)
            return false;
        else
            return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const {
        if (cards.length() != 2)
            return NULL;
        else {
            Slash *slash = new Slash(Card::SuitToBeDecided, -1);
            slash->addSubcards(cards);
            slash->setSkillName(objectName());
            return slash;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return player->hasFlag("thhuanlong") && Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return player->hasFlag("thhuanlong") && pattern == "slash";
    }
};

class ThHuanlong: public TriggerSkill {
public:
    ThHuanlong(): TriggerSkill("thhuanlong") {
        events << EventPhaseStart << EventPhaseChanging;
        view_as_skill = new ThHuanlongViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (triggerEvent == EventPhaseStart && TriggerSkill::triggerable(player) && player->getPhase() == Player::Play)
            return QStringList(objectName());
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive && player->hasFlag("thhuanlong"))
                room->setPlayerFlag(player, "-thhuanlong");
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

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        forever {
            QStringList choices;
            if (!player->hasFlag("thhuanlong1"))
                choices << "thhuanlong1";
            if (!player->hasFlag("thhuanlong2"))
                choices << "thhuanlong2";
            if (!player->hasFlag("thhuanlong"))
                choices << "thhuanlong3";
            if (choices.isEmpty()) break;
            if (choices.length() < 3)
                choices << "cancel";
            QString choice = room->askForChoice(player, "thhuanlong", choices.join("+"));
            if (choice == "cancel") break;
            LogMessage log;
            log.type = "#ThHuanlong";
            log.from = player;
            log.arg = "thhuanlong";
            log.arg2 = choice.right(1);
            room->sendLog(log);
            if (choice == "thhuanlong3")
                choice = "thhuanlong";
            room->setPlayerFlag(player, choice);
        }
        return false;
    }
};

class ThHuanlongTargetMod: public TargetModSkill {
public:
    ThHuanlongTargetMod(): TargetModSkill("#thhuanlong-target") {
    }

    virtual int getResidueNum(const Player *from, const Card *) const{
        if (from->hasFlag("thhuanlong2"))
            return 1;
        else
            return 0;
    }
};

class ThHuanlongMaxCards: public MaxCardsSkill {
public:
    ThHuanlongMaxCards(): MaxCardsSkill("#thhuanlong-max-cards") {
    }

    virtual int getExtra(const Player *target) const {
        int delta = 0;
        if (target->hasFlag("thhuanlong1"))
            --delta;
        if (target->hasFlag("thhuanlong2"))
            --delta;
        if (target->hasFlag("thhuanlong"))
            --delta;
        return delta;
    }
};

ThChiyingCard::ThChiyingCard() {
}

bool ThChiyingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool ThChiyingCard::targetFixed() const{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE)
        return true;

    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetFixed();
}

bool ThChiyingCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    const Card *card = NULL;
    if (!user_string.isEmpty())
        card = Sanguosha->cloneCard(user_string.split("+").first());
    return card && card->targetsFeasible(targets, Self);
}

const Card *ThChiyingCard::validate(CardUseStruct &cardUse) const{
    cardUse.m_isOwnerUse = false;
    ServerPlayer *user = cardUse.from;
    Room *room = user->getRoom();
    QStringList names = user_string.split("+");
    if (names.contains("slash")) names << "fire_slash" << "thunder_slash";

    ServerPlayer *current = room->getCurrent();
    if (!current || current->getPhase() == Player::NotActive || current->isKongcheng()) {
        room->setPlayerFlag(user, "Global_ThChiyingFailed");
        return NULL;
    }

    room->setPlayerFlag(user, "thchiying");

    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "thchiying";
    room->sendLog(log);

    int card_id = room->askForCardChosen(user, current, "h", "thchiying");
    room->showCard(current, card_id);

    const Card *card = Sanguosha->getCard(card_id);
    if (names.contains(card->objectName()))
        if (user->askForSkillInvoke("thchiying_use", "use"))
            return card;
    if (card->getSuit() == Club)
        if (user->askForSkillInvoke("thchiying_use", "change")) {
            QString name = room->askForChoice(user, "thchiying", names.join("+"));
            Card *c = Sanguosha->cloneCard(name);
            c->addSubcard(card);
            c->setSkillName("thchiying");
            return c;
        }
    return NULL;
}

const Card *ThChiyingCard::validateInResponse(ServerPlayer *user) const{
    Room *room = user->getRoom();
    QStringList names = user_string.split("+");
    if (names.contains("slash")) names << "fire_slash" << "thunder_slash";

    ServerPlayer *current = room->getCurrent();
    if (!current || current->getPhase() == Player::NotActive || current->isKongcheng()) {
        room->setPlayerFlag(user, "Global_ThChiyingFailed");
        return NULL;
    }

    room->setPlayerFlag(user, "thchiying");

    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = user;
    log.arg = "thchiying";
    room->sendLog(log);

    int card_id = room->askForCardChosen(user, current, "h", "thchiying");
    room->showCard(current, card_id);

    const Card *card = Sanguosha->getCard(card_id);
    if (names.contains(card->objectName()))
        if (user->askForSkillInvoke("thchiying_use", "use"))
            return card;
    if (card->getSuit() == Club)
        if (user->askForSkillInvoke("thchiying_use", "change")) {
            QString name = room->askForChoice(user, "thchiying", names.join("+"));
            Card *c = Sanguosha->cloneCard(name);
            c->addSubcard(card);
            c->setSkillName("thchiying");
            return c;
        }
    return NULL;
}

class ThChiyingViewAsSkill: public ZeroCardViewAsSkill {
public:
    ThChiyingViewAsSkill(): ZeroCardViewAsSkill("thchiying") {
    }

    virtual bool isEnabledAtPlay(const Player *) const {
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const {
        if (player->getPhase() != Player::NotActive || player->hasFlag("Global_ThChiyingFailed") || player->hasFlag("thchiying"))
            return false;
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_RESPONSE_USE)
            return false;
        if (pattern == "slash")
            return true;
        else if (pattern == "peach")
            return player->getMark("Global_PreventPeach") == 0;
        else if (pattern.contains("analeptic"))
            return true;
        return false;
    }

    virtual const Card *viewAs() const{
        ThChiyingCard *card = new ThChiyingCard;
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern == "peach+analeptic" && Self->getMark("Global_PreventPeach") > 0)
            pattern = "analeptic";
        card->setUserString(pattern);
        return card;
    }
};

class ThChiying: public TriggerSkill {
public:
    ThChiying(): TriggerSkill("thchiying") {
        events << CardAsked << EventPhaseChanging;
        view_as_skill = new ThChiyingViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive) {
                foreach (ServerPlayer *p, room->getAlivePlayers()) {
                    if (p->hasFlag("thchiying"))
                        room->setPlayerFlag(p, "-thchiying");
                }
            }
            return QStringList();
        }
        ServerPlayer *current = room->getCurrent();
        if (!TriggerSkill::triggerable(player) || player->hasFlag("thchiying")) return QStringList();
        if (!current || current == player || current->getPhase() == Player::NotActive || current->isKongcheng()) return QStringList();
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_RESPONSE_USE) return QStringList();
        QString pattern = data.toStringList().first();
        if (pattern == "jink")
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName(), data)) {
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        room->setPlayerFlag(player, "thchiying");
        ServerPlayer *current = room->getCurrent();
        int card_id = room->askForCardChosen(player, current, "h", "thchiying");
        room->showCard(current, card_id);

        const Card *card = Sanguosha->getCard(card_id);
        if (card->objectName() == "jink")
            if (player->askForSkillInvoke("thchiying_use", "use")) {
                room->provide(card);
                return true;
            }
        if (card->getSuit() == Card::Club)
            if (player->askForSkillInvoke("thchiying_use", "change")) {
                Card *c = Sanguosha->cloneCard("jink");
                c->addSubcard(card);
                c->setSkillName("thchiying");
                room->provide(c);
                return true;
            }
        return false;
    }
};

ThXuezhongCard::ThXuezhongCard() {
    target_fixed = true;
}

void ThXuezhongCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const {
    QList<int> card_ids = room->getNCards(subcardsLength(), false);
    CardMoveReason reason(CardMoveReason::S_REASON_TURNOVER, source->objectName(), "thxuezhong", QString());
    CardsMoveStruct move(card_ids, NULL, Player::PlaceTable, reason);
    room->moveCardsAtomic(move, true);
    room->getThread()->delay();
    QList<int> spade, other;
    foreach (int id, card_ids) {
        if (Sanguosha->getCard(id)->getSuit() == Spade)
            spade << id;
        else
            other << id;
    }
    DummyCard *dummy = new DummyCard;
    if (!other.isEmpty()) {
        dummy->addSubcards(other);
        source->obtainCard(dummy);
    }
    dummy->clearSubcards();
    if (!spade.isEmpty()) {
        dummy->addSubcards(spade);
        ServerPlayer *target = room->askForPlayerChosen(source, room->getOtherPlayers(source), objectName());
        target->obtainCard(dummy);
        if (target->getHandcardNum() > source->getHandcardNum())
            target->turnOver();
    }
    delete dummy;
}

class ThXuezhong: public ViewAsSkill {
public:
    ThXuezhong(): ViewAsSkill("thxuezhong") {
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        return !Self->isJilei(to_select) && to_select->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const {
        if (cards.isEmpty())
            return NULL;
        ThXuezhongCard *card = new ThXuezhongCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->canDiscard(player, "he") && !player->hasUsed("ThXuezhongCard");
    }
};

ThLunminCard::ThLunminCard() {
    target_fixed = true;
}

void ThLunminCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const {
    source->drawCards(1);
}

class ThLunmin: public OneCardViewAsSkill {
public:
    ThLunmin(): OneCardViewAsSkill("thlunmin") {
        filter_pattern = ".!";
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        return player->usedTimes("ThLunminCard") < 3;
    }

    virtual const Card *viewAs(const Card *originalCard) const {
        ThLunminCard *card = new ThLunminCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class ThYupan: public TriggerSkill {
public:
    ThYupan(): TriggerSkill("thyupan") {
        events << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *player) const {
        foreach (ServerPlayer *p, player->getRoom()->getAlivePlayers())
            if (p->isWounded())
                return TriggerSkill::triggerable(player)
                    && player->getPhase() == Player::Finish
                    && player->getMark("thyupan") == 0xF;
        return false;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getAlivePlayers())
            if (p->isWounded())
                targets << p;
        if (targets.isEmpty()) return false;
        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@thyupan", true, true);
        if (target) {
            room->broadcastSkillInvoke(objectName());
            player->tag["ThYupanTarget"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        ServerPlayer *target = player->tag["ThYupanTarget"].value<ServerPlayer *>();
        player->tag.value("ThYupanTarget");
        if (target)
            room->recover(target, RecoverStruct(player));
        return false;
    }
};

class ThYupanRecord: public TriggerSkill {
public:
    ThYupanRecord(): TriggerSkill("#thyupan-record") {
        events << EventPhaseChanging << PreCardUsed << CardResponded << BeforeCardsMove;
        frequency = Compulsory;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive)
                if (player->getMark("thyupan") > 0)
                    room->setPlayerMark(player, "thyupan", 0);
        } else {
            if (!player->isAlive() || player->getPhase() == Player::NotActive) return QStringList();
            if (triggerEvent == PreCardUsed || triggerEvent == CardResponded) {
                const Card *card = NULL;
                if (triggerEvent == PreCardUsed) {
                    card = data.value<CardUseStruct>().card;
                } else {
                    CardResponseStruct resp = data.value<CardResponseStruct>();
                    if (resp.m_isUse)
                        card = resp.m_card;
                }
                if (!card) return QStringList();
                recordThYupanCardSuit(room, player, card);
            } else if (triggerEvent == BeforeCardsMove) {
                CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
                if (player != move.from
                    || ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) != CardMoveReason::S_REASON_DISCARD))
                    return QStringList();
                foreach (int id, move.card_ids) {
                    const Card *c = Sanguosha->getCard(id);
                    recordThYupanCardSuit(room, player, c);
                }
            }
        }
        return QStringList();
    }

private:
    void recordThYupanCardSuit(Room *room, ServerPlayer *player, const Card *card) const{
        if (player->getMark("thyupan") == 0xF) return;
        int suitID = (1 << int(card->getSuit()));
        int mark = player->getMark("thyupan");
        if ((mark & suitID) == 0)
            room->setPlayerMark(player, "thyupan", mark | suitID);
    }
};

class ThJiuzhang: public FilterSkill {
public:
    ThJiuzhang(): FilterSkill("thjiuzhang") {
    }

    static WrappedCard *changeToNine(int cardId) {
        WrappedCard *new_card = Sanguosha->getWrappedCard(cardId);
        new_card->setSkillName("thjiuzhang");
        new_card->setNumber(9);
        new_card->setModified(true);
        return new_card;
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->getNumber() > 9;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        return changeToNine(originalCard->getEffectiveId());
    }
};

ThShushuCard::ThShushuCard() {
    target_fixed = true;
    will_throw = false;
    handling_method = MethodNone;
}

void ThShushuCard::onUse(Room *room, const CardUseStruct &card_use) const {
    LogMessage log;
    log.type = "$ThShushu";
    log.from = card_use.from;
    log.arg = "thshushu";
    log.card_str = IntList2StringList(subcards).join("+");
    room->sendLog(log);

    CardMoveReason reason(CardMoveReason::S_REASON_OVERRIDE, card_use.from->objectName(), "thshushu", QString());
    QList<CardsMoveStruct> moves;
    foreach (int id, subcards) {
        CardsMoveStruct move(id, NULL, Player::PlaceTable, reason);
        moves.append(move);
    }
    room->moveCardsAtomic(moves, true);
}

class ThShushuViewAsSkill: public ViewAsSkill {
public:
    ThShushuViewAsSkill(): ViewAsSkill("thshushu") {
        response_pattern = "@@thshushu";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        int num = Self->getMark("thshushu");
        int n = 0;
        foreach (const Card *c, selected)
            n += c->getNumber();
        if (n >= num) return false;
        return !Self->isJilei(to_select) && to_select->getNumber() <= num - n;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const {
        int num = Self->getMark("thshushu");
        int n = 0;
        foreach (const Card *c, cards)
            n += c->getNumber();
        if (num != n)
            return NULL;
        ThShushuCard *card = new ThShushuCard;
        card->addSubcards(cards);
        return card;
    }
};

class ThShushu: public TriggerSkill {
public:
    ThShushu(): TriggerSkill("thshushu") {
        events << TargetConfirming;
        view_as_skill = new ThShushuViewAsSkill;
    }

    virtual TriggerList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        TriggerList skill_list;
        CardUseStruct use = data.value<CardUseStruct>();
        if ((use.card->isKindOf("BasicCard") || use.card->isNDTrick()) && !use.card->isKindOf("ExNihilo") && use.card->getNumber() > 0) {
            QList<int> ids;
            if (use.card->isVirtualCard())
                ids = use.card->getSubcards();
            else
                ids << use.card->getEffectiveId();
            if (ids.length() > 0) {
                bool all_place_table = true;
                foreach (int id, ids) {
                    if (room->getCardPlace(id) != Player::PlaceTable) {
                        all_place_table = false;
                        break;
                    }
                }
                if (all_place_table) {
                    foreach (ServerPlayer *p, room->findPlayersBySkillName(objectName())) {
                        if (p->isNude())
                            continue;
                        int dis = p->distanceTo(player);
                        if (dis >= 0 && dis <= 2)
                            skill_list.insert(p, QStringList(objectName()));
                    }
                }
            }
        }
            
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        CardUseStruct use = data.value<CardUseStruct>();
        room->setPlayerMark(player, "thshushu", use.card->getNumber());
        const Card *card = room->askForUseCard(player, "@@thshushu", "@thshushu", -1, Card::MethodNone);
        room->setPlayerMark(player, "thshushu", 0);
        if (card)
            player->tag["ThShushuCard"] = QVariant::fromValue(card);
        return card;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const {
        const Card *card = player->tag["ThShushuCard"].value<const Card *>();
        player->tag.remove("ThShushuCard");
        if (card) {
            CardUseStruct use = data.value<CardUseStruct>();
            room->obtainCard(player, use.card);
            Card *new_card = Sanguosha->cloneCard(use.card->getClassName());
            new_card->addSubcards(card->getSubcards());
            new_card->setSkillName(objectName());
            new_card->deleteLater();
            use.card = new_card;
            data = QVariant::fromValue(use);
        }
        return false;
    }
};

ThFenglingCard::ThFenglingCard() {
    target_fixed = true;
}

class ThFenglingViewAsSkill: public ViewAsSkill {
public:
    ThFenglingViewAsSkill(): ViewAsSkill("thfengling") {
        response_pattern = "@@thfengling";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        int n = 0;
        foreach (const Card *cd, selected)
            n += cd->getNumber();
        if (n >= 9)
            return false;
        return n + to_select->getNumber() <= 9;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        int n = 0;
        foreach (const Card *cd, cards)
            n += cd->getNumber();
        if (n != 9)
            return NULL;

        ThFenglingCard *card = new ThFenglingCard;
        card->addSubcards(cards);
        return card;
    }
};

class ThFengling: public TriggerSkill {
public:
    ThFengling(): TriggerSkill("thfengling") {
        events << Dying;
        frequency = Limited;
        limit_mark = "@fengling";
        view_as_skill = new ThFenglingViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        if (!TriggerSkill::triggerable(player))
            return QStringList();
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.who != player || player->getMark("@fengling") == 0)
            return QStringList();
        if (player->getHp() > 0)
            return QStringList();
        return QStringList(objectName());
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player->askForSkillInvoke(objectName())) {
            room->broadcastSkillInvoke(objectName());
            room->removePlayerMark(player, "@fengling");
            room->addPlayerMark(player, "@fenglingused");
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        player->drawCards(5, objectName());

        while (room->askForUseCard(player, "@@thfengling", "@thfengling", -1, Card::MethodDiscard)) {
            room->recover(player, RecoverStruct(player));
            if (!player->isWounded())
                break;
            if (!player->canDiscard(player, "he"))
                break;
        }
        return false;
    }
};

ThYingshiCard::ThYingshiCard() {
    mute = true;
    target_fixed = true;
}

const Card *ThYingshiCard::validate(CardUseStruct &card_use) const {
    ServerPlayer *player = card_use.from;
    Room *room = player->getRoom();
    int n = player->usedTimes("ThYingshiCard") + 1;
    QList<ServerPlayer *> targets;
    foreach (ServerPlayer *p, room->getAllPlayers())
        if (player->distanceTo(p) == n)
            targets << p;
    if (targets.isEmpty()) {
        room->setPlayerFlag(player, "Global_ThYingshiFailed");
        return NULL;
    }

    player->setFlags("ThYingshiUse");
    bool use = room->askForUseSlashTo(player, targets, "@thyingshi:::" + QString::number(n));
    if (!use) {
        player->setFlags("-ThYingshiUse");
        room->setPlayerFlag(player, "Global_ThYingshiFailed");
    } else {
        if (n != 1)
            room->setPlayerMark(player, "@yingshi" + QString::number(n - 1), 0);
        room->addPlayerHistory(player, "ThYingshiCard");
        room->setPlayerMark(player, "@yingshi" + QString::number(n), 1);
    }
    return NULL;
}

class ThYingshiViewAsSkill: public ZeroCardViewAsSkill {
public:
    ThYingshiViewAsSkill(): ZeroCardViewAsSkill("thyingshi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const {
        int n = player->usedTimes("ThYingshiCard");
        if (n >= 3 || player->hasFlag("Global_ThYingshiFailed"))
            return false;
        foreach (const Player *p, player->getAliveSiblings())
            if (player->distanceTo(p) == n + 1 && player->canSlash(p))
                return true;
        return false;
    }

    virtual const Card *viewAs() const {
        return new ThYingshiCard;
    }
};

class ThYingshi: public TriggerSkill {
public:
    ThYingshi(): TriggerSkill("thyingshi") {
        events << EventPhaseChanging;
        view_as_skill = new ThYingshiViewAsSkill;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return QStringList();
        room->setPlayerMark(player, "@yingshi1", 0);
        room->setPlayerMark(player, "@yingshi2", 0);
        room->setPlayerMark(player, "@yingshi3", 0);
        return QStringList();
    }
};

class ThZanghun: public TriggerSkill {
public:
    ThZanghun(): TriggerSkill("thzanghun") {
        events << Damage;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!TriggerSkill::triggerable(player)) return QStringList();
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card->isKindOf("Slash"))
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

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const {
        player->drawCards(1);
        room->addPlayerMark(player, "thzanghun");
        return false;
    }
};

class ThZanghunClear: public TriggerSkill {
public:
    ThZanghunClear(): TriggerSkill("#thzanghun-clear") {
        events << EventPhaseChanging;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const {
        if (!player || player->getMark("thzanghun") == 0) return QStringList();
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return QStringList();
        room->setPlayerMark(player, "thzanghun", 0);
        return QStringList();
    }
};

class ThZanghunDistance: public DistanceSkill {
public:
    ThZanghunDistance(): DistanceSkill("#thzanghun-distance") {
    }

    virtual int getCorrect(const Player *from, const Player *) const {
        return from->getMark("thzanghun");
    }
};

TouhouSPPackage::TouhouSPPackage()
    :Package("touhou-sp")
{
    General *sp001 = new General(this, "sp001", "kaze");
    sp001->addSkill(new ThFanshi);

    General *sp002 = new General(this, "sp002", "hana", 4, false);
    sp002->addSkill(new ThNichang);
    sp002->addSkill(new Skill("thqimen", Skill::Compulsory));
    sp002->addSkill(new ThGuanjia);

    General *sp003 = new General(this, "sp003", "yuki", 3);
    sp003->addSkill(new ThShenshi);
    sp003->addSkill(new ThJiefan);

    General *sp004 = new General(this, "sp004", "tsuki", 3);
    sp004->addSkill(new ThChuangshi);
    sp004->addSkill(new SlashNoDistanceLimitSkill("thchuangshi"));
    related_skills.insertMulti("thchuangshi", "#thchuangshi-slash-ndl");
    sp004->addSkill(new ThGaotian);

    General *sp005 = new General(this, "sp005", "kaze");
    sp005->addSkill(new ThWanling);
    sp005->addSkill(new ThZuibu);

    General *sp006 = new General(this, "sp006", "hana");
    sp006->addSkill(new ThModao);

    General *sp007 = new General(this, "sp007", "yuki", 3);
    sp007->addSkill(new ThMengsheng);
    sp007->addSkill(new ThMengshengClear);
    sp007->addSkill(new ThMengshengInvalidity);
    related_skills.insertMulti("thmengsheng", "#thmengsheng-clear");
    related_skills.insertMulti("thmengsheng", "#thmengsheng-inv");
    sp007->addSkill(new ThQixiang);

    General *sp008 = new General(this, "sp008", "tsuki");
    sp008->addSkill(new ThHuanlong);
    sp008->addSkill(new ThHuanlongTargetMod);
    sp008->addSkill(new ThHuanlongMaxCards);
    related_skills.insertMulti("thhuanlong", "#thhuanlong-target");
    related_skills.insertMulti("thhuanlong", "#thhuanlong-max-cards");

    General *sp009 = new General(this, "sp009", "kaze", 3);
    sp009->addSkill(new ThChiying);
    sp009->addSkill(new ThXuezhong);

    General *sp010 = new General(this, "sp010", "hana", 3);
    sp010->addSkill(new ThLunmin);
    sp010->addSkill(new ThYupan);
    sp010->addSkill(new ThYupanRecord);
    related_skills.insertMulti("thyupan", "#thyupan-record");

    General *sp011 = new General(this, "sp011", "yuki");
    sp011->addSkill(new ThJiuzhang);
    sp011->addSkill(new ThShushu);
    sp011->addSkill(new ThFengling);

    General *sp012 = new General(this, "sp012", "tsuki");
    sp012->addSkill(new ThYingshi);
    sp012->addSkill(new ThZanghun);
    sp012->addSkill(new ThZanghunClear);
    sp012->addSkill(new ThZanghunDistance);
    related_skills.insertMulti("thzanghun", "#thzanghun-clear");
    related_skills.insertMulti("thzanghun", "#thzanghun-distance");

    /*General *sp999 = new General(this, "sp999", "te", 5, true, true);
    sp999->addSkill("jibu");
    sp999->addSkill(new Skill("thfeiniang", Skill::Compulsory));*/

    addMetaObject<ThChiyingCard>();
    addMetaObject<ThXuezhongCard>();
    addMetaObject<ThLunminCard>();
    addMetaObject<ThShushuCard>();
    addMetaObject<ThFenglingCard>();
    addMetaObject<ThYingshiCard>();
}

ADD_PACKAGE(TouhouSP)
