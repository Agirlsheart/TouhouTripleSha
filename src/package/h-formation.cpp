#include "h-formation.h"
#include "general.h"
#include "standard.h"
#include "standard-equips.h"
#include "maneuvering.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"
#include "settings.h"
#include "jsonutils.h"

class Ziliang: public TriggerSkill {
public:
    Ziliang(): TriggerSkill("ziliang") {
        events << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        foreach (ServerPlayer *dengai, room->getAllPlayers()) {
            if (!TriggerSkill::triggerable(dengai) || !player->isAlive()) break;
            if (dengai->getPile("ikyindiepile").isEmpty()) continue;
            if (!room->askForSkillInvoke(dengai, objectName(), data)) continue;
            room->fillAG(dengai->getPile("ikyindiepile"), dengai);
            int id = room->askForAG(dengai, dengai->getPile("ikyindiepile"), false, objectName());
            room->clearAG(dengai);
            if (player == dengai) {
                LogMessage log;
                log.type = "$MoveCard";
                log.from = player;
                log.to << player;
                log.card_str = QString::number(id);
                room->sendLog(log);
            }
            room->obtainCard(player, id);
        }
        return false;
    }
};

class Tianfu: public TriggerSkill {
public:
    Tianfu(): TriggerSkill("tianfu") {
        events << EventPhaseStart << EventPhaseChanging;
    }

    virtual int getPriority(TriggerEvent) const{
        return 4;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == EventPhaseStart && player->getPhase() == Player::RoundStart) {
            QList<ServerPlayer *> jiangweis = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *jiangwei, jiangweis) {
                if (jiangwei->isAlive() && (player == jiangwei || player->isAdjacentTo(jiangwei))
                    && room->askForSkillInvoke(player, objectName(), QVariant::fromValue(jiangwei))) {
                    if (player != jiangwei) {
                        room->notifySkillInvoked(jiangwei, objectName());
                        LogMessage log;
                        log.type = "#InvokeOthersSkill";
                        log.from = player;
                        log.to << jiangwei;
                        log.arg = objectName();
                        room->sendLog(log);
                    }
                    jiangwei->addMark(objectName());
                    room->acquireSkill(jiangwei, "ikxuanying");
                }
            }
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive) return false;
            foreach (ServerPlayer *p, room->getAllPlayers()) {
                if (p->getMark(objectName()) > 0) {
                    p->setMark(objectName(), 0);
                    room->detachSkillFromPlayer(p, "ikxuanying", false, true);
                }
            }
        }
        return false;
    }
};

ShangyiCard::ShangyiCard() {
}

bool ShangyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void ShangyiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    ServerPlayer *player = effect.to;
    if (!effect.from->isKongcheng())
        room->showAllCards(effect.from, player);
    QStringList choicelist;
    if (!effect.to->isKongcheng())
        choicelist.append("handcards");
    if (room->getMode() == "04_1v3" || room->getMode() == "06_3v3") {
        ;
    } else if (room->getMode() == "06_XMode") {
        QStringList backup = player->tag["XModeBackup"].toStringList();
        if (backup.length() > 0)
            choicelist.append("remainedgenerals");
    } else if (room->getMode() == "02_1v1") {
        QStringList list = player->tag["1v1Arrange"].toStringList();
        if (list.length() > 0)
            choicelist.append("remainedgenerals");
    } else if (Config.EnableBasara) {
        QString hidden_generals = player->property("basara_generals").toString();
        if (!hidden_generals.isEmpty())
            choicelist.append("generals");
    } else if (!player->isLord()) {
        choicelist.append("role");
    }
    if (choicelist.isEmpty()) return;
    QString choice = room->askForChoice(effect.from, "shangyi", choicelist.join("+"), QVariant::fromValue(player));

    LogMessage log;
    log.type = "$ShangyiView";
    log.from = effect.from;
    log.to << effect.to;
    log.arg = "shangyi:" + choice;
    room->sendLog(log, room->getOtherPlayers(effect.from));

    if (choice == "handcards") {
        QList<int> ids;
        foreach (const Card *card, player->getHandcards()) {
            if (card->isBlack())
                ids << card->getEffectiveId();
        }

        int card_id = room->doGongxin(effect.from, player, ids, "shangyi");
        if (card_id == -1) return;
        effect.from->tag.remove("shangyi");
        CardMoveReason reason(CardMoveReason::S_REASON_DISMANTLE, effect.from->objectName(), QString(), "shangyi", QString());
        room->throwCard(Sanguosha->getCard(card_id), reason, effect.to, effect.from);
    } else if (choice == "remainedgenerals") {
        QStringList list;
        if (room->getMode() == "02_1v1")
            list = player->tag["1v1Arrange"].toStringList();
        else if (room->getMode() == "06_XMode")
            list = player->tag["XModeBackup"].toStringList();
        foreach (QString name, list) {
            LogMessage log;
            log.type = "$ShangyiViewRemained";
            log.from = effect.from;
            log.to << player;
            log.arg = name;
            room->sendLog(log, effect.from);
        }
        Json::Value arr(Json::arrayValue);
        arr[0] = QSanProtocol::Utils::toJsonString("shangyi");
        arr[1] = QSanProtocol::Utils::toJsonArray(list);
        room->doNotify(effect.from, QSanProtocol::S_COMMAND_VIEW_GENERALS, arr);
    } else if (choice == "generals") {
        QStringList list = player->property("basara_generals").toString().split("+");
        foreach (QString name, list) {
            LogMessage log;
            log.type = "$ShangyiViewUnknown";
            log.from = effect.from;
            log.to << player;
            log.arg = name;
            room->sendLog(log, effect.from);
        }
        Json::Value arg(Json::arrayValue);
        arg[0] = QSanProtocol::Utils::toJsonString("shangyi");
        arg[1] = QSanProtocol::Utils::toJsonArray(list);
        room->doNotify(effect.from, QSanProtocol::S_COMMAND_VIEW_GENERALS, arg);
    } else if (choice == "role") {
        Json::Value arg(Json::arrayValue);
        arg[0] = QSanProtocol::Utils::toJsonString(player->objectName());
        arg[1] = QSanProtocol::Utils::toJsonString(player->getRole());
        room->doNotify(effect.from, QSanProtocol::S_COMMAND_SET_EMOTION, arg);

        LogMessage log;
        log.type = "$ViewRole";
        log.from = effect.from;
        log.to << player;
        log.arg = player->getRole();
        room->sendLog(log, effect.from);
    }
}

class Shangyi: public ZeroCardViewAsSkill {
public:
    Shangyi(): ZeroCardViewAsSkill("shangyi") {
    }

    virtual const Card *viewAs() const{
        return new ShangyiCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShangyiCard");
    }
};

class Niaoxiang: public TriggerSkill {
public:
    Niaoxiang(): TriggerSkill("niaoxiang") {
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash") && use.from->isAlive()) {
            QVariantList jink_list = use.from->tag["Jink_" + use.card->toString()].toList();
            for (int i = 0; i < use.to.length(); i++) {
                ServerPlayer *to = use.to.at(i);
                if (to->isAlive() && to->isAdjacentTo(player) && to->isAdjacentTo(use.from)
                    && room->askForSkillInvoke(player, objectName(), QVariant::fromValue(to))) {
                    room->broadcastSkillInvoke(objectName());
                    if (jink_list.at(i).toInt() == 1)
                        jink_list.replace(i, QVariant(2));
                }
            }
            use.from->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
        }

        return false;
    }
};

class Yicheng: public TriggerSkill {
public:
    Yicheng(): TriggerSkill("yicheng") {
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card->isKindOf("Slash")) return false;
        foreach (ServerPlayer *p, use.to) {
            if (room->askForSkillInvoke(player, objectName(), QVariant::fromValue(p))) {
                p->drawCards(1, objectName());
                if (p->isAlive() && p->canDiscard(p, "he"))
                    room->askForDiscard(p, objectName(), 1, 1, false, true);
            }
            if (!player->isAlive())
                break;
        }
        return false;
    }
};

class Qianhuan: public TriggerSkill {
public:
    Qianhuan(): TriggerSkill("qianhuan") {
        events << Damaged << TargetConfirming;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == Damaged && player->isAlive()) {
            ServerPlayer *yuji = room->findPlayerBySkillName(objectName());
            if (yuji && room->askForSkillInvoke(player, objectName(), "choice:" + yuji->objectName())) {
                room->broadcastSkillInvoke(objectName());
                if (yuji != player) {
                    room->notifySkillInvoked(yuji, objectName());
                    LogMessage log;
                    log.type = "#InvokeOthersSkill";
                    log.from = player;
                    log.to << yuji;
                    log.arg = objectName();
                    room->sendLog(log);
                }

                int id = room->drawCard();
                Card::Suit suit = Sanguosha->getCard(id)->getSuit();
                bool duplicate = false;
                foreach (int card_id, yuji->getPile("sorcery")) {
                    if (Sanguosha->getCard(card_id)->getSuit() == suit) {
                        duplicate = true;
                        break;
                    }
                }
                yuji->addToPile("sorcery", id);
                if (duplicate) {
                    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
                    room->throwCard(Sanguosha->getCard(id), reason, NULL);
                }
            }
        } else if (triggerEvent == TargetConfirming) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->getTypeId() == Card::TypeEquip || use.card->getTypeId() == Card::TypeSkill)
                return false;
            if (use.to.length() != 1) return false;
            ServerPlayer *yuji = room->findPlayerBySkillName(objectName());
            if (!yuji || yuji->getPile("sorcery").isEmpty()) return false;
            if (room->askForSkillInvoke(yuji, objectName(), data)) {
                room->broadcastSkillInvoke(objectName());
                room->notifySkillInvoked(yuji, objectName());
                if (yuji == player || room->askForChoice(player, objectName(), "accept+reject", data) == "accept") {
                    QList<int> ids = yuji->getPile("sorcery");
                    int id = -1;
                    if (ids.length() > 1) {
                        room->fillAG(ids, yuji);
                        id = room->askForAG(yuji, ids, false, objectName());
                        room->clearAG(yuji);
                    } else {
                        id = ids.first();
                    }
                    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
                    room->throwCard(Sanguosha->getCard(id), reason, NULL);

                    LogMessage log;
                    if (use.from) {
                        log.type = "$CancelTarget";
                        log.from = use.from;
                    } else {
                        log.type = "$CancelTargetNoUser";
                    }
                    log.to = use.to;
                    log.arg = use.card->objectName();
                    room->sendLog(log);

                    use.to.clear();
                    data = QVariant::fromValue(use);
                } else {
                    LogMessage log;
                    log.type = "#IkBianshengReject";
                    log.from = player;
                    log.to << yuji;
                    log.arg = objectName();
                    room->sendLog(log);
                }
            }
        }
        return false;
    }
};

HFormationPackage::HFormationPackage()
    : Package("h_formation")
{
    General *heg_dengai = new General(this, "heg_dengai", "wei"); // WEI 015 G
    heg_dengai->addSkill("ikyindie");
    heg_dengai->addSkill(new Ziliang);

    General *heg_jiangwei = new General(this, "heg_jiangwei", "shu"); // SHU 012 G
    heg_jiangwei->addSkill("iktiaoxin");
    heg_jiangwei->addSkill(new Tianfu);

    General *jiangqin = new General(this, "jiangqin", "wu"); // WU 017
    jiangqin->addSkill(new Shangyi);
    jiangqin->addSkill(new Niaoxiang);

    General *heg_xusheng = new General(this, "heg_xusheng", "wu"); // WU 020
    heg_xusheng->addSkill(new Yicheng);

    General *heg_yuji = new General(this, "heg_yuji", "qun", 3); // QUN 011 G
    heg_yuji->addSkill(new Qianhuan);

    addMetaObject<ShangyiCard>();
}

ADD_PACKAGE(HFormation)
