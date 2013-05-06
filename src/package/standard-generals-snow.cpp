#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "standard-generals.h"
#include "maneuvering.h"
#include "ai.h"

ZhihengCard::ZhihengCard(){
    target_fixed = true;
    will_throw = false;
    handling_method = Card::MethodDiscard;
}

void ZhihengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->throwCard(this, source);
    if(source->isAlive())
        room->drawCards(source, subcards.length());
}

class Zhiheng:public ViewAsSkill{
public:
    Zhiheng():ViewAsSkill("zhiheng"){

    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *) const{
        return selected.length() < Self->getMaxHp();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        ZhihengCard *zhiheng_card = new ZhihengCard;
        zhiheng_card->addSubcards(cards);
        zhiheng_card->setSkillName(objectName());
        return zhiheng_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ZhihengCard");
    }
};

class Jiyuan: public TriggerSkill{
public:
    Jiyuan():TriggerSkill("jiyuan$"){
        events << TargetConfirmed << HpRecover;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill("jiyuan");
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == TargetConfirmed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Peach") && use.from && use.from->getKingdom() == "wu"
                && player != use.from) {
                room->setCardFlag(use.card, "jiyuan");
            }
        } else if (triggerEvent == HpRecover) {
            RecoverStruct rec = data.value<RecoverStruct>();
            if (rec.card && rec.card->hasFlag("jiyuan")) {

                LogMessage log;
                log.type = "#JiyuanExtraRecover";
                log.from = player;
                log.to << rec.who;
                log.arg = objectName();
                room->sendLog(log);

                rec.recover++;
                data = QVariant::fromValue(rec);
            }
        }

        return false;
    }
};

class Kuipo: public OneCardViewAsSkill{
public:
    Kuipo():OneCardViewAsSkill("kuipo"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->isBlack();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Dismantlement *dismantlement = new Dismantlement(originalCard->getSuit(), originalCard->getNumber());
        dismantlement->addSubcard(originalCard->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};

class Biju: public TriggerSkill {
public:
    Biju(): TriggerSkill("biju") {
        events << EventPhaseChanging << CardUsed << CardResponded;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *lvmeng, QVariant &data) const{
        if (event == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.who == lvmeng && change.to == Player::Discard) {
                if (!lvmeng->hasFlag("biju_use_slash") && lvmeng->askForSkillInvoke(objectName())) {
                    room->broadcastSkillInvoke(objectName());
                    lvmeng->skip(Player::Discard);
                }
            }
        } else if (lvmeng->getPhase() == Player::Play) {
            CardStar card = NULL;
            if (event == CardUsed)
            {
                CardUseStruct use = data.value<CardUseStruct>();
                if (use.from != lvmeng)
                    return false;
                card = use.card;
            }
            else
            {
                CardResponseStruct resp = data.value<CardResponseStruct>();
                if (resp.m_who != lvmeng)
                    return false;
                card = resp.m_card;
            }

            if (card && card->isKindOf("Slash"))
                room->setPlayerFlag(lvmeng, "biju_use_slash");
        }

        return false;
    }
};

class Pojian: public TriggerSkill {
public:
    Pojian(): TriggerSkill("pojian") {
        events << EventPhaseStart << CardUsed << CardResponded;
        frequency = Wake;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const {
        if (player->getMark("@pojian") > 0)
            return false;
        if (triggerEvent == CardUsed || triggerEvent == CardResponded)
        {
            if (triggerEvent == CardUsed)
            {
                CardUseStruct use = data.value<CardUseStruct>();
                if (use.from != player)
                    return false;
                if (use.card->isKindOf("BasicCard") || use.card->isKindOf("TrickCard"))
                    player->addMark("pojian_count");
            }
            else
            {
                CardResponseStruct resp = data.value<CardResponseStruct>();
                if (resp.m_who != player || !resp.m_isUse)
                    return false;
                if (resp.m_card->isKindOf("BasicCard") || resp.m_card->isKindOf("TrickCard"))
                    player->addMark("pojian_count");
            }
        }
        else
        {
            if (data.value<PlayerStar>() != player)
                return false;
            if (player->getPhase() == Player::RoundStart)
            {
                room->setPlayerMark(player, "pojian_count", 0);
                return false;
            }
            else if (player->getPhase() != Player::Finish)
                return false;
            if (player->getMark("pojian_count") < 5)
                return false;
            int n = player->getMark("pojian_count");
            LogMessage log;
            log.type = "#PojianWake";
            log.from = player;
            log.arg = QString::number(n);
            log.arg2 = objectName();
            room->sendLog(log);

            room->broadcastSkillInvoke(objectName());

            player->gainMark("@pojian");

            room->loseMaxHp(player);
            RecoverStruct recover;
            recover.who = player;
            room->recover(player, recover);

            room->acquireSkill(player, "qinghua");
        }

        return false;
    }
};

QinghuaCard::QinghuaCard() {
}

bool QinghuaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void QinghuaCard::onEffect(const CardEffectStruct &effect) const {
    Room *room = effect.from->getRoom();
    const Card *card1 = room->askForCardShow(effect.to, effect.from, "qinghua");
    room->showCard(effect.to, card1->getEffectiveId());
    const Card *card2 = room->askForCard(effect.from, ".." + card1->getSuitString().left(1).toUpper(),
                                         "@QinghuaDiscard:::" + card1->getSuitString(), QVariant(), "qinghua");
    if (card2)
    {
        RecoverStruct recover;
        recover.who = effect.from;
        room->recover(effect.from, recover);
        room->recover(effect.to, recover);
    }
}

class Qinghua: public ZeroCardViewAsSkill {
public:
    Qinghua(): ZeroCardViewAsSkill("qinghua") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("QinghuaCard");
    }

    virtual const Card *viewAs() const{
        return new QinghuaCard;
    }
};

KurouCard::KurouCard() {
    target_fixed = true;
}

void KurouCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->loseHp(source);
    if (source->isAlive())
        room->drawCards(source, 2);
}

class Kurou: public ZeroCardViewAsSkill {
public:
    Kurou(): ZeroCardViewAsSkill("kurou") {
    }

    virtual const Card *viewAs() const{
        return new KurouCard;
    }
};

class Zaiqi: public TriggerSkill {
public:
    Zaiqi(): TriggerSkill("zaiqi") {
        events << HpRecover;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        RecoverStruct recover = data.value<RecoverStruct>();
        if (player->hasFlag("dying") && player->getPhase() == Player::Play)
            while(recover.recover--)
            {
                if (!player->askForSkillInvoke(objectName()))
                    break;
                player->drawCards(1);
            }

        return false;
    }
};

GuidengCard::GuidengCard(){
    once = true;
    handling_method = MethodNone;
    will_throw = false;
}

void GuidengCard::onEffect(const CardEffectStruct &effect) const{
    if (subcardsLength() == 0)
        return ;
    ServerPlayer *player = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = player->getRoom();

    int card_id = getSubcards().first();
    room->broadcastSkillInvoke("guideng");
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target, "guideng");

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->getThread()->delay();
    target->obtainCard(card);

    if(card->getSuit() != suit){
        DamageStruct damage;
        damage.card = NULL;
        damage.from = player;
        damage.to = target;

        room->damage(damage);
    }
}

class Guideng:public OneCardViewAsSkill{
public:
    Guideng():OneCardViewAsSkill("guideng"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("GuidengCard");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        GuidengCard *card = new GuidengCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Chenhong:public DrawCardsSkill{
public:
    Chenhong():DrawCardsSkill("chenhong"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        Room *room = player->getRoom();
        if(room->askForSkillInvoke(player, objectName())){
            room->broadcastSkillInvoke(objectName());
            return n + 1;
        }else
            return n;
    }
};

class Wanmei: public OneCardViewAsSkill{
public:
    Wanmei():OneCardViewAsSkill("wanmei"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Indulgence *indulgence = new Indulgence(originalCard->getSuit(), originalCard->getNumber());
        indulgence->addSubcard(originalCard->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

class Xuanhuo: public TriggerSkill{
public:
    Xuanhuo():TriggerSkill("xuanhuo"){
        events << TargetConfirming;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *target = NULL;
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (p->getMark("TargetConfirming") > 0)
            {
                target = p;
                break;
            }
        if (!target || target != player)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();

        if (use.card->isKindOf("Slash") && !player->isNude() && room->alivePlayerCount() > 2){
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            players.removeOne(use.from);

            QList<ServerPlayer *> victims;
            foreach(ServerPlayer *p, players)
                if (player->inMyAttackRange(p) && !use.from->isProhibited(p, use.card))
                    victims << p;

            if(!victims.isEmpty())
                if (room->askForCard(player, "..", "@xuanhuo" + use.from->objectName(), QVariant(), objectName()))
                {
                    ServerPlayer *victim = room->askForPlayerChosen(player, victims, objectName());
                    use.to.insert(use.to.indexOf(player), victim);
                    use.to.removeOne(player);

                    data = QVariant::fromValue(use);
                    return true;
                }
        }

        return false;
    }
};

class Bujie: public TriggerSkill{
public:
    Bujie():TriggerSkill("bujie"){
        events << CardsMoveOneTime;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
        if(move->from == player && move->from_places.contains(Player::PlaceHand) && player->isKongcheng()){
            if(room->askForSkillInvoke(player, objectName(), data)){
                room->broadcastSkillInvoke(objectName());
                player->drawCards(1);
            }
        }
        return false;
    }
};

YuanheCard::YuanheCard(){
}

void YuanheCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    source->drawCards(2);
    target->drawCards(2);
    room->askForDiscard(source, "yuanhe", 2, 2, false, true);
    room->askForDiscard(target, "yuanhe", 2, 2, false, true);
}

class Yuanhe: public OneCardViewAsSkill{
public:
    Yuanhe():OneCardViewAsSkill("yuanhe"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return(!player->hasUsed("YuanheCard"));
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isRed() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        YuanheCard *card = new YuanheCard;
        card->addSubcard(originalCard);

        return card;
    }
};

YuluCard::YuluCard(){
    once = true;
    mute = true;
}

bool YuluCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->isMale() && to_select->isWounded();
}

void YuluCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);

    room->broadcastSkillInvoke("yulu");
}

class Yulu: public ViewAsSkill{
public:
    Yulu():ViewAsSkill("yulu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("YuluCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if(selected.length() > 1)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        YuluCard *yulu_card = new YuluCard();
        yulu_card->addSubcards(cards);

        return yulu_card;
    }
};

class Cuimeng: public TriggerSkill{
public:
    Cuimeng():TriggerSkill("cuimeng"){
        events << CardsMoveOneTime;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
        if(move->from == player && move->from_places.contains(Player::PlaceEquip)){
            bool invoke = false;
            foreach(Player::Place place, move->from_places)
                if(place == Player::PlaceEquip){
                    invoke = true;
                    break;
                }

            if(invoke && room->askForSkillInvoke(player, objectName())){
                room->broadcastSkillInvoke(objectName());
                player->drawCards(2);
            }
        }

        return false;
    }
};

LiangbanCard::LiangbanCard(){
    mute = true;
}

void LiangbanCard::onEffect(const CardEffectStruct &effect) const{
    int x = qMax(effect.from->getLostHp(), 1);
    Room *room = effect.from->getRoom();

    bool good = false;
    if(x == 1){
        room->broadcastSkillInvoke("liangban");

        effect.to->drawCards(1);
        room->askForDiscard(effect.to, "liangban", 1, 1, false, true);
        good = true;
    }else{
        QString choice = room->askForChoice(effect.from, "liangban", "d1tx+dxt1");
        if(choice == "d1tx"){
            room->broadcastSkillInvoke("liangban");

            effect.to->drawCards(1);
            x = qMin(x, effect.to->getCardCount(true));
            room->askForDiscard(effect.to, "liangban", x, x, false, true);
            good = false;
        }else{
            room->broadcastSkillInvoke("liangban");

            effect.to->drawCards(x);
            room->askForDiscard(effect.to, "liangban", 1, 1, false, true);
            good = true;
        }
    }

    if(good)
        room->setEmotion(effect.to, "good");
    else
        room->setEmotion(effect.to, "bad");
}

class LiangbanViewAsSkill: public ZeroCardViewAsSkill{
public:
    LiangbanViewAsSkill():ZeroCardViewAsSkill("liangban"){
    }

    virtual const Card *viewAs() const{
        return new LiangbanCard;
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@liangban";
    }
};

class Liangban: public TriggerSkill {
public:
    Liangban(): TriggerSkill("liangban"){
        events << EventPhaseStart;
        default_choice = "d1tx";
        view_as_skill = new LiangbanViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const {
        return target != NULL && TriggerSkill::triggerable(target)
                && target->getPhase() == Player::Start;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const {
        if (data.value<PlayerStar>() == player)
            room->askForUseCard(player, "@@liangban", "@liangban");

        return false;
    }
};

JilianCard::JilianCard() {
    will_throw = false;
    mute = true;
    handling_method = Card::MethodNone;
}

bool JilianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (!targets.isEmpty() || to_select == Self)
        return false;

    return to_select->getHandcardNum() == Self->getMark("jilian");
}

void JilianCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const{
    room->moveCardTo(this, targets.first(), Player::PlaceHand, false);
}

class JilianViewAsSkill: public ViewAsSkill {
public:
    JilianViewAsSkill(): ViewAsSkill("jilian") {
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (to_select->isEquipped())
            return false;

        int length = Self->getHandcardNum() / 2;
        return selected.length() < length;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != Self->getHandcardNum() / 2)
            return NULL;

        JilianCard *card = new JilianCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@jilian!";
    }
};

class JilianGive: public TriggerSkill {
public:
    JilianGive(): TriggerSkill("#jilian-give") {
        events << AfterDrawNCards;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *lusu, QVariant &) const{
        if (lusu->hasFlag("jilian")) {
            lusu->setFlags("-jilian");

            if (lusu->getHandcardNum() <= 5)
                return false;            

            QList<ServerPlayer *> other_players = room->getOtherPlayers(lusu);
            int least = 1000;
            foreach (ServerPlayer *player, other_players)
                least = qMin(player->getHandcardNum(), least);
            room->setPlayerMark(lusu, "jilian", least);
            bool used = room->askForUseCard(lusu, "@@jilian!", "@jilian", -1, Card::MethodNone);

            if (!used) {
                // force lusu to give his half cards
                ServerPlayer *beggar = NULL;
                foreach (ServerPlayer *player, other_players) {
                    if (player->getHandcardNum() == least) {
                        beggar = player;
                        break;
                    }
                }

                int n = lusu->getHandcardNum() / 2;
                QList<int> to_give = lusu->handCards().mid(0, n);
                JilianCard *jilian_card = new JilianCard;
                foreach (int card_id, to_give)
                    jilian_card->addSubcard(card_id);
                QList<ServerPlayer *> targets;
                targets << beggar;
                jilian_card->use(room, lusu, targets);
                delete jilian_card;
            }
        }

        return false;
    }
};

class Jilian: public DrawCardsSkill {
public:
    Jilian(): DrawCardsSkill("#jilian") {
    }

    virtual int getDrawNum(ServerPlayer *lusu, int n) const{
        Room *room = lusu->getRoom();
        if (room->askForSkillInvoke(lusu, "jilian")) {
            room->broadcastSkillInvoke("jilian");
            lusu->setFlags("jilian");
            return n + 2;
        } else
            return n;
    }
};

QixiCard::QixiCard() {
}

bool QixiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if (to_select == Self)
        return false;

    if (targets.isEmpty())
        return true;

    if (targets.length() == 1) {
        return subcards.length() == qAbs(to_select->getHandcardNum() - targets.first()->getHandcardNum());
    }

    return false;
}

bool QixiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void QixiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *a = targets.at(0);
    ServerPlayer *b = targets.at(1);

    int n1 = a->getHandcardNum();
    int n2 = b->getHandcardNum();

    QList<CardsMoveStruct> exchangeMove;
    CardsMoveStruct move1;
    move1.card_ids = a->handCards();
    move1.to = b;
    move1.to_place = Player::PlaceHand;
    CardsMoveStruct move2;
    move2.card_ids = b->handCards();
    move2.to = a;
    move2.to_place = Player::PlaceHand;
    exchangeMove.push_back(move1);
    exchangeMove.push_back(move2);
    room->moveCards(exchangeMove, false);

    LogMessage log;
    log.type = "#Qixi";
    log.from = a;
    log.to << b;
    log.arg = QString::number(n1);
    log.arg2 = QString::number(n2);
    room->sendLog(log);
    room->getThread()->delay();
}

class Qixi: public ViewAsSkill {
public:
    Qixi(): ViewAsSkill("qixi") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("QixiCard");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *) const{
        return selected.length() < 3;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        QixiCard *acard = new QixiCard;
        if (cards.length() != 0)
            acard->addSubcards(cards);
        return acard;
    }
};

ZhihuiCard::ZhihuiCard()
{
}

void ZhihuiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->setPlayerFlag(effect.to, "ZhihuiTarget");
    DamageStruct damage = effect.from->tag.value("ZhihuiDamage").value<DamageStruct>();
    damage.to = effect.to;
    damage.transfer = true;
    room->damage(damage);
}

class ZhihuiViewAsSkill: public OneCardViewAsSkill{
public:
    ZhihuiViewAsSkill():OneCardViewAsSkill("zhihui"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zhihui";
    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped() && to_select->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        ZhihuiCard *zhihuiCard = new ZhihuiCard;
        zhihuiCard->addSubcard(originalCard);
        return zhihuiCard;
    }
};

class Zhihui: public TriggerSkill{
public:
    Zhihui():TriggerSkill("zhihui"){
        events << DamageInflicted << DamageComplete;

        view_as_skill = new ZhihuiViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == DamageInflicted && TriggerSkill::triggerable(player) && !player->isKongcheng())
        {
            DamageStruct damage = data.value<DamageStruct>();

            player->tag["ZhihuiDamage"] = QVariant::fromValue(damage);
            if(room->askForUseCard(player, "@@zhihui", "@zhihui-card")){
                return true;
            }
        }else if(triggerEvent == DamageComplete && player->hasFlag("ZhihuiTarget") && player->isAlive()){
            room->setPlayerFlag(player, "-ZhihuiTarget");
            player->drawCards(player->getLostHp(), false);
        }
        return false;
    }
};

class Chiqiu: public FilterSkill{
public:
    Chiqiu():FilterSkill("chiqiu"){

    }

    static WrappedCard *changeToHeart(int cardId){
        WrappedCard *new_card = Sanguosha->getWrappedCard(cardId);
        new_card->setSkillName("chiqiu");
        new_card->setSuit(Card::Heart);
        new_card->setModified(true);
        return new_card;
    }

    virtual bool viewFilter(const Card* to_select) const{
        return to_select->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        return changeToHeart(originalCard->getEffectiveId());
    }
};

JianmieCard::JianmieCard(){
    once = true;
    will_throw = false;
    handling_method = Card::MethodPindian;
}

bool JianmieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void JianmieCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    bool success = source->pindian(targets.first(), "jianmie", this);
    if(success){
        room->setPlayerFlag(source, "jianmie_success");
    }else{
        room->setPlayerCardLimitation(source, "use", "Slash", true);
    }
}

class JianmieViewAsSkill: public OneCardViewAsSkill{
public:
    JianmieViewAsSkill():OneCardViewAsSkill("jianmie"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("JianmieCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *card = new JianmieCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Jianmie: public TriggerSkill{
public:
    Jianmie():TriggerSkill("jianmie"){
        view_as_skill = new JianmieViewAsSkill;
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->hasFlag("jianmie_success");
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *player, QVariant &data) const{
        if(data.toString() == objectName())
            room->setPlayerFlag(player, "-jianmie_success");

        return false;
    }
};

class JianmieTargetMod: public TargetModSkill {
public:
    JianmieTargetMod(): TargetModSkill("#jianmie-target") {
        frequency = NotFrequent;
    }

    virtual int getResidueNum(const Player *from, const Card *) const{
        if (from->hasSkill("jianmie") && from->hasFlag("jianmie_success"))
            return 1;
        else
            return 0;
    }

    virtual int getDistanceLimit(const Player *from, const Card *) const{
        if (from->hasSkill("jianmie") && from->hasFlag("jianmie_success"))
            return 1000;
        else
            return 0;
    }

    virtual int getExtraTargetNum(const Player *from, const Card *) const{
        if (from->hasSkill("jianmie") && from->hasFlag("jianmie_success"))
            return 1;
        else
            return 0;
    }
};

class Susheng: public TriggerSkill{
public:
    Susheng():TriggerSkill("susheng"){
        events << PostHpReduced << AskForPeachesDone;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        if(triggerEvent == PostHpReduced && player->getHp() < 1){
            if(room->askForSkillInvoke(player, objectName())){
                room->setTag("Susheng", player->objectName());
                room->broadcastSkillInvoke(objectName());
                const QList<int> &susheng = player->getPile("sushengpile");

                int need = 1 - player->getHp(); // the susheng cards that should be turned over
                int n = need - susheng.length();
                if(n > 0){
                    QList<int> card_ids = room->getNCards(n);
                    player->addToPile("sushengpile", card_ids);
                }
                const QList<int> &sushengnew = player->getPile("sushengpile");
                QList<int> duplicate_numbers;

                QSet<int> numbers;
                foreach(int card_id, sushengnew){
                    const Card *card = Sanguosha->getCard(card_id);
                    int number = card->getNumber();

                    if(numbers.contains(number)){
                        duplicate_numbers << number;
                    }else
                        numbers << number;
                }

                if(duplicate_numbers.isEmpty()){
                    room->setTag("Susheng", QVariant());
                    return true;
                }
                else
                {
                    room->setTag("Susheng", QVariant());
                    player->clearOnePrivatePile("sushengpile");
                }
            }
        }else if(triggerEvent == AskForPeachesDone){
            const QList<int> &susheng = player->getPile("sushengpile");

            if(player->getHp() > 0)
                return false;
            if(room->getTag("Susheng").toString() != player->objectName())
                return false;
            room->setTag("Susheng", QVariant());
            QList<int> duplicate_numbers;

            QSet<int> numbers;
            foreach(int card_id, susheng){
                const Card *card = Sanguosha->getCard(card_id);
                int number = card->getNumber();

                if(numbers.contains(number) && !duplicate_numbers.contains(number)){
                    duplicate_numbers << number;
                }else
                    numbers << number;
            }

            if(duplicate_numbers.isEmpty()){
                room->broadcastSkillInvoke(objectName());
                room->setPlayerFlag(player, "-dying");
                return true;
            }else{
                LogMessage log;
                log.type = "#SushengDuplicate";
                log.from = player;
                log.arg = QString::number(duplicate_numbers.length());
                room->sendLog(log);

                for(int i=0; i<duplicate_numbers.length(); i++){
                    int number = duplicate_numbers.at(i);

                    LogMessage log;
                    log.type = "#SushengDuplicateGroup";
                    log.from = player;
                    log.arg = QString::number(i+1);
                    log.arg2 = Card::Number2String(number);
                    room->sendLog(log);

                    foreach(int card_id, susheng){
                        const Card *card = Sanguosha->getCard(card_id);
                        if(card->getNumber() == number){
                            LogMessage log;
                            log.type = "$SushengDuplicateItem";
                            log.from = player;
                            log.card_str = QString::number(card_id);
                            room->sendLog(log);
                        }
                    }
                }
            }
        }
        return false;
    }
};

class SushengRemove: public TriggerSkill{
public:
    SushengRemove():TriggerSkill("#susheng-remove"){
        events << HpRecover << EventLoseSkill;
    }

    virtual int getPriority() const{
        return -1;
    }

    static void Remove(ServerPlayer *player){
        Room *room = player->getRoom();
        QList<int> susheng(player->getPile("sushengpile"));

        CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "susheng", QString());
        int need = 1 - player->getHp();
        if(need <= 0){
            // clear all the susheng cards
            foreach(int card_id, susheng) {

                LogMessage log;
                log.type = "$SushengRemove";
                log.from = player;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
            }
        }else{
            int to_remove = susheng.length() - need;

            for(int i = 0; i < to_remove; i++){
                room->fillAG(susheng);
                int card_id = room->askForAG(player, susheng, false, "susheng");

                LogMessage log;
                log.type = "$SushengRemove";
                log.from = player;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                susheng.removeOne(card_id);
                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
                room->broadcastInvoke("clearAG");
            }
        }
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == HpRecover && TriggerSkill::triggerable(player)
                && player->getPile("sushengpile").length() > 0)
            Remove(player);
        else if(triggerEvent == EventLoseSkill && data.toString() == "susheng"){
            player->clearOnePrivatePile("susheng");
            if(player->getHp() <= 0)
                room->enterDying(player, NULL);
        }
        return false;
    }
};

class Eli: public TriggerSkill{
public:
    Eli():TriggerSkill("eli"){
        events << CardsMoveOneTime << EventPhaseChanging << EventPhaseEnd;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *srcplayer = NULL;
        if (triggerEvent == EventPhaseEnd)
            srcplayer = data.value<PlayerStar>();
        else if (triggerEvent == EventPhaseChanging)
            srcplayer = data.value<PhaseChangeStruct>().who;
        else
            srcplayer = player;

        if (srcplayer == NULL || srcplayer != player)
            return false;

        if(triggerEvent == EventPhaseChanging){
            player->setMark("eli", 0);
        }
        else if(triggerEvent == CardsMoveOneTime)
        {
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if(move->from != player)
                return false;

            if(move->to_place == Player::DiscardPile && player->getPhase() == Player::Discard)
                player->setMark("eli", player->getMark("eli") + move->card_ids.length());
        }
        else if(triggerEvent == EventPhaseEnd && player->getHp() <= 0 && player->getMark("eli") >= 2 && player->askForSkillInvoke(objectName())){
            SavageAssault *nanman = new SavageAssault(Card::NoSuitNoColor, 0);
            nanman->setSkillName(objectName());
            CardUseStruct use;
            use.from = player;
            use.card = nanman;

            foreach(ServerPlayer *p, room->getOtherPlayers(player))
                if(!player->isProhibited(p, nanman))
                    use.to << p;

            if(use.to.isEmpty())
                return false;

            room->useCard(use, true);
        }

        return false;
    }
};

class Heyi: public TriggerSkill{
public:
    Heyi():TriggerSkill("heyi"){
        events << TargetConfirmed;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.from->objectName() == player->objectName() || use.to.contains(player)){
            if(use.card->isKindOf("Duel") || (use.card->isKindOf("Slash") && use.card->isRed())){
                if(player->askForSkillInvoke(objectName(), data)){
                    room->broadcastSkillInvoke(objectName());
                    player->drawCards(1);
                }
            }
        }

        return false;
    }
};

class Chizhu: public TriggerSkill {
public:
    Chizhu(): TriggerSkill("chizhu") {
        events << EventPhaseStart;
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const {
        return target != NULL && TriggerSkill::triggerable(target)
                && target->getMark("@chizhu") == 0
                && target->getPhase() == Player::Start
                && target->getHp() <= 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const {
        if (data.value<PlayerStar>() != player)
            return false;

        LogMessage log;
        log.type = "#ChizhuWake";
        log.from = player;
        log.arg = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        //room->broadcastInvoke("animate", "lightbox:$chizhu:5000");
        //room->getThread()->delay(5000);
        player->gainMark("@chizhu");
        room->loseMaxHp(player);

        room->acquireSkill(player, "liangban");
        room->acquireSkill(player, "chenhong");

        return false;
    }
};

BianshengCard::BianshengCard(){
    mute = true;
    will_throw = false;
    m_skillName = "biansheng_pindian";
    handling_method = Card::MethodPindian;
}

bool BianshengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("biansheng") && to_select != Self
            && !to_select->isKongcheng() && !to_select->hasFlag("BianshengInvoked");
}

void BianshengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *lord = targets.first();
    room->setPlayerFlag(lord, "BianshengInvoked");
    if(lord->getMark("chizhu") > 0 &&
       room->askForChoice(lord, "biansheng_pindian", "accept+reject") == "reject")
    {
        //room->broadcastSkillInvoke("sunce_zhiba", 4);
        return;
    }

    //room->broadcastSkillInvoke("sunce_zhiba", 1);
    source->pindian(lord, "biansheng_pindian", this);
    QList<ServerPlayer *> lords;
    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach(ServerPlayer *p, players){
        if(p->hasLordSkill("biansheng") && !p->hasFlag("BianshengInvoked")){
            lords << p;
        }
    }
    if(lords.empty())
        room->setPlayerFlag(source, "ForbidBiansheng");
}

class BianshengPindian: public OneCardViewAsSkill{
public:
    BianshengPindian():OneCardViewAsSkill("biansheng_pindian"){
        attached_lord_skill = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getKingdom() == "wu" && !player->isKongcheng() && !player->hasFlag("ForbidBiansheng");
    }

    virtual bool viewFilter(const Card* to_select) const{
        return ! to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        BianshengCard *card = new BianshengCard;
        card->addSubcard(originalCard);

        return card;
    }
};

class Biansheng:public TriggerSkill{
public:
    Biansheng():TriggerSkill("biansheng$"){
        events << EventPhaseStart << EventPhaseEnd << EventPhaseChanging << Pindian;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        ServerPlayer *player = NULL;
        if (triggerEvent != EventPhaseChanging)
            player = data.value<PlayerStar>();
        else
            player = data.value<PhaseChangeStruct>().who;
        
        if (player == NULL && triggerEvent != Pindian)
            return false;

        if (triggerEvent == EventPhaseEnd && player->hasSkill("biansheng_pindian"))
            room->detachSkillFromPlayer(player, "biansheng_pindian", true);
        else if (triggerEvent == EventPhaseStart)
        {
            bool can_invoke = false;
            foreach(ServerPlayer *p, room->getOtherPlayers(player))
                if (p->hasLordSkill("biansheng"))
                {
                    can_invoke = true;
                    break;
                }
            if (can_invoke && player->getPhase() == Player::Play && !player->hasSkill("biansheng_pindian") && player->getKingdom() == "wu")
                room->attachSkillToPlayer(player, "biansheng_pindian");
        }
        else if (triggerEvent == EventPhaseChanging)
        {
            PhaseChangeStruct phase_change = data.value<PhaseChangeStruct>();
            if (phase_change.from != Player::Play)
                  return false;
            if(player->hasFlag("ForbidBiansheng")){
                room->setPlayerFlag(player, "-ForbidBiansheng");
            }
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach(ServerPlayer *p, players){
                if(p->hasFlag("BianshengInvoked")){
                    room->setPlayerFlag(p, "-BianshengInvoked");
                }
            }
        }
        else if(triggerEvent == Pindian)
        {
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason != "biansheng_pindian" || !pindian->to->hasLordSkill(objectName()))
                return false;
            if(!pindian->isSuccess())
                if (room->askForChoice(pindian->to, "biansheng", "yes+no") == "yes") {                    
                    pindian->to->obtainCard(pindian->from_card);
                    pindian->to->obtainCard(pindian->to_card);
                }
        }

        return false;
    }
};

JibanCard::JibanCard(){
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool JibanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() || to_select == Self)
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void JibanCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *player = effect.from;
    player->getRoom()->moveCardTo(this, player, effect.to, Player::PlaceEquip,
        CardMoveReason(CardMoveReason::S_REASON_USE, player->objectName(), "jiban", QString()));

    LogMessage log;
    log.type = "$JibanEquip";
    log.from = effect.to;
    log.card_str = Sanguosha->getCard(subcards.first())->getEffectIdString();
    player->getRoom()->sendLog(log);

    player->drawCards(1);
}

class Jiban: public OneCardViewAsSkill{
public:
    Jiban():OneCardViewAsSkill("jiban"){

    }

    virtual bool viewFilter(const Card* to_select) const{
        return !to_select->isEquipped() && to_select->getTypeId() == Card::TypeEquip;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        JibanCard *jiban_card = new JibanCard();
        jiban_card->addSubcard(originalCard);
        return jiban_card;
    }
};

class Jizhou: public TriggerSkill{
public:
    Jizhou():TriggerSkill("jizhou"){
        events << CardsMoveOneTime;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *current = room->getCurrent();
        if (player == current)
            return false;

        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();

        if((move->reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) != CardMoveReason::S_REASON_DISCARD)
            return false;

        if(current->getPhase() == Player::Discard){
            QVariantList guzheng = player->tag["Jizhou"].toList();

            foreach (int card_id, move->card_ids)
                guzheng << card_id;

            player->tag["Jizhou"] = guzheng;
        }

        return false;
    }
};

class JizhouGet: public TriggerSkill{
public:
    JizhouGet():TriggerSkill("#jizhou-get"){
        events << EventPhaseEnd;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *splayer, QVariant &data) const{
        ServerPlayer *player = data.value<PlayerStar>();
        if(player->isDead())
            return false;

        QVariantList guzheng_cards = splayer->tag["Jizhou"].toList();
        splayer->tag.remove("Jizhou");

        QList<int> cards;
        foreach(QVariant card_data, guzheng_cards){
            int card_id = card_data.toInt();
            if(room->getCardPlace(card_id) == Player::DiscardPile)
                cards << card_id;
        }

        if(cards.isEmpty())
            return false;

        if(splayer->askForSkillInvoke("jizhou", cards.length())){
            room->broadcastSkillInvoke("jizhou");
            room->fillAG(cards, splayer);

            int to_back = room->askForAG(splayer, cards, false, objectName());
            player->obtainCard(Sanguosha->getCard(to_back));

            cards.removeOne(to_back);

            splayer->invoke("clearAG");

            CardsMoveStruct move;
            move.card_ids = cards;
            move.to = splayer;
            move.to_place = Player::PlaceHand;
            QList<CardsMoveStruct> moves;
            moves.append(move);
            room->moveCardsAtomic(moves, true);
        }

        return false;
    }
};

class Leilan: public TriggerSkill {
public:
    Leilan(): TriggerSkill("leilan") {
        events << CardsMoveOneTime << EventPhaseStart << EventPhaseEnd;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (triggerEvent == CardsMoveOneTime)
        {
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if (move->from == player && move->from_places.contains(Player::PlaceEquip))
            {
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);
                player->gainMark("@jilei");
            }
        }
        
        if (triggerEvent == EventPhaseStart)
        {
            if (data.value<PlayerStar>()->getPhase() == Player::Finish)
                while (player->getMark("@jilei") > 0)
                {
                    LogMessage log;
                    log.type = "#TriggerSkill";
                    log.from = player;
                    log.arg = objectName();
                    room->sendLog(log);

                    player->loseMark("@jilei");
                    QStringList choicelist;
                    QList<ServerPlayer *> targets1;
                    ThunderSlash *slash = new ThunderSlash(Card::NoSuitNoColor, 0);
                    slash->setSkillName(objectName());
                    foreach (ServerPlayer *target, room->getAlivePlayers())
                        if (player->canSlash(target, slash, false))
                            targets1 << target;

                    if (!targets1.isEmpty())
                        choicelist << "slash";

                    QList<ServerPlayer *> targets2;
                    foreach (ServerPlayer *p, room->getOtherPlayers(player))
                        if (player->distanceTo(p) <= 1)
                            targets2 << p;

                    if (!targets2.isEmpty())
                        choicelist << "damage";

                    choicelist << "draw";

                    QString choice = room->askForChoice(player, objectName(), choicelist.join("+"));
                    if (choice == "slash")
                    {
                        ServerPlayer *target = room->askForPlayerChosen(player, targets1, objectName());

                        CardUseStruct card_use;
                        card_use.card = slash;
                        card_use.from = player;
                        card_use.to << target;
                        room->useCard(card_use, false);
                    }
                    else if (choice == "damage")
                    {
                        ServerPlayer *target = room->askForPlayerChosen(player, targets2, objectName());

                        DamageStruct damage;
                        damage.from = player;
                        damage.to = target;
                        damage.nature = DamageStruct::Thunder;
                        room->damage(damage);
                    }
                    else
                    {
                        player->drawCards(1);
                        room->askForDiscard(player, objectName(), 1, 1);
                    }
                }
        }
        
        if (player->getPhase() != Player::Discard)
            return false;

        if (triggerEvent == CardsMoveOneTime)
        {
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if (move->from == player && move->to_place == Player::DiscardPile
                && (move->reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)
                player->setMark("leilan", player->getMark("leilan") + move->card_ids.size());
        }
        else if (triggerEvent == EventPhaseStart && data.value<PlayerStar>() == player)
            player->setMark("leilan", 0);
        else if (triggerEvent == EventPhaseEnd && data.value<PlayerStar>() == player
                   && player->getMark("leilan") >= 2)
        {
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            player->gainMark("@jilei");
        }

        return false;
    }
};

class Yuanfa: public TriggerSkill {
public:
    Yuanfa(): TriggerSkill("yuanfa") {
        events << Dying;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.who->isKongcheng()) return false;
        if (dying.who->getHp() < 1 && player->askForSkillInvoke(objectName(), data)) {
            const Card *card = NULL;
            if (dying.who == player)
                card = room->askForCardShow(dying.who, player, objectName());
            else {
                int card_id = room->askForCardChosen(player, dying.who, "h", objectName());
                card = Sanguosha->getCard(card_id);
            }

            room->showCard(dying.who, card->getEffectiveId());

            if (card->getTypeId() != Card::TypeBasic) {
                if (!dying.who->isHuyin(card))
                    room->throwCard(card, dying.who);

                room->broadcastSkillInvoke(objectName());

                RecoverStruct recover;
                recover.who = player;
                room->recover(dying.who, recover);
            }
        }
        return dying.who->getHp() > 0;
    }
};

GuanjuCard::GuanjuCard() {
}

void GuanjuCard::swapEquip(ServerPlayer *first, ServerPlayer *second) const{
    Room *room = first->getRoom();

    QList<int> equips1, equips2;
    foreach (const Card *equip, first->getEquips())
        equips1.append(equip->getId());
    foreach (const Card *equip, second->getEquips())
        equips2.append(equip->getId());

    QList<CardsMoveStruct> exchangeMove;
    CardsMoveStruct move1;
    move1.card_ids = equips1;
    move1.to = second;
    move1.to_place = Player::PlaceEquip;
    CardsMoveStruct move2;
    move2.card_ids = equips2;
    move2.to = first;
    move2.to_place = Player::PlaceEquip;
    exchangeMove.push_back(move2);
    exchangeMove.push_back(move1);
    room->moveCards(exchangeMove, false);
}

bool GuanjuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

bool GuanjuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    switch (targets.length()) {
    case 0: return true;
    case 1: {
            int n1 = targets.first()->getEquips().length();
            int n2 = to_select->getEquips().length();
            return qAbs(n1 - n2) <= Self->getLostHp();
        }
    default:
        return false;
    }
}

void GuanjuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    swapEquip(targets.first(), targets[1]);

    LogMessage log;
    log.type = "#GuanjuSwap";
    log.from = source;
    log.to = targets;
    room->sendLog(log);
}

class Guanju: public ZeroCardViewAsSkill {
public:
    Guanju(): ZeroCardViewAsSkill("guanju") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("GuanjuCard");
    }

    virtual const Card *viewAs() const{
        return new GuanjuCard;
    }
};

class Zhongqu: public TriggerSkill{
public:
    Zhongqu():TriggerSkill("zhongqu"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->isDead())
            return false;

        if(damage.card && damage.card->isKindOf("Slash")){
            if(room->askForSkillInvoke(player, objectName(), data)){
                int x = qMin(5, damage.to->getHp());
                if (x >= 3)
                    room->broadcastSkillInvoke(objectName(), 2);
                else
                    room->broadcastSkillInvoke(objectName(), 1);
                damage.to->drawCards(x);
                damage.to->turnOver();
            }
        }
        return false;
    }
};

class LingpaoViewAsSkill: public OneCardViewAsSkill {
public:
    LingpaoViewAsSkill(): OneCardViewAsSkill("lingpao") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        FireSlash *slash = new FireSlash(Card::NoSuitNoColor, 0);
        slash->deleteLater();
        return slash->isAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE
               && pattern == "slash";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->objectName() == "slash";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *acard = new FireSlash(originalCard->getSuit(), originalCard->getNumber());
        acard->addSubcard(originalCard->getId());
        acard->setSkillName(objectName());
        return acard;
    }
};

class Lingpao: public TriggerSkill {
public:
    Lingpao(): TriggerSkill("lingpao") {
        events << DamageDone << CardFinished;
        view_as_skill = new LingpaoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if (event == DamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->isKindOf("Slash") && damage.card->getSkillName() == objectName())
                damage.from->tag["LingpaoSlash"] = QVariant::fromValue(damage.card);
        } else if (TriggerSkill::triggerable(player) && data.value<CardUseStruct>().from == player
                   && !player->tag.value("LingpaoSlash").isNull()
                   && data.value<CardUseStruct>().card == player->tag.value("LingpaoSlash").value<CardStar>()) {
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            player->tag.remove("LingpaoSlash");
            room->broadcastSkillInvoke("lingpao", 2);
            room->loseHp(player, 1);
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const {
        return 1;
    }
};

class LingpaoTargetMod: public TargetModSkill {
public:
    LingpaoTargetMod(): TargetModSkill("#lingpao-target") {
        frequency = NotFrequent;
    }

    virtual int getExtraTargetNum(const Player *from, const Card *card) const{
        if (from->hasSkill("lingpao") && card->isKindOf("FireSlash"))
            return 1;
        else
            return 0;
    }
};

XiaozuiCard::XiaozuiCard() {
    will_throw = false;
    target_fixed = true;
    handling_method = Card::MethodNone;
}

void XiaozuiCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const{
    source->addToPile("xiaozuipile", this);
}

class XiaozuiViewAsSkill: public ViewAsSkill {
public:
    XiaozuiViewAsSkill(): ViewAsSkill("xiaozui") {
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@xiaozui";
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *to_select) const{
        return to_select->isKindOf("Slash");
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() == 0)
            return NULL;

        Card *acard = new XiaozuiCard;
        acard->addSubcards(cards);
        acard->setSkillName(objectName());
        return acard;
    }
};

class Xiaozui: public TriggerSkill {
public:
    Xiaozui(): TriggerSkill("xiaozui") {
        events << EventPhaseStart << AskForPeaches;
        view_as_skill = new XiaozuiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *chengpu, QVariant &data) const{
        if (triggerEvent == EventPhaseStart && data.value<PlayerStar>() == chengpu
            && chengpu->getPhase() == Player::Finish && !chengpu->isKongcheng()
            && chengpu->getPile("xiaozuipile").isEmpty())
        {
            room->askForUseCard(chengpu, "@@xiaozui", "@xiaozui", -1, Card::MethodNone);
        }
        else if (triggerEvent == AskForPeaches && !chengpu->getPile("xiaozuipile").isEmpty()) {
            DyingStruct dying = data.value<DyingStruct>();
            while (dying.who->getHp() < 1 && !chengpu->getPile("xiaozuipile").isEmpty()
                   && chengpu->askForSkillInvoke(objectName(), data)) {
                QList<int> cards = chengpu->getPile("xiaozuipile");
                room->fillAG(cards, chengpu);
                int card_id = room->askForAG(chengpu, cards, true, objectName());
                room->broadcastInvoke("clearAG");
                if (card_id != -1) {
                    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "xiaozui", QString());
                    room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
                    Peach *peach = new Peach(Card::NoSuitNoColor, 0);
                    peach->setSkillName(objectName());
                    CardUseStruct use;
                    use.card = peach;
                    use.from = dying.who;
                    use.to << dying.who;
                    room->useCard(use);
                }
            }
        }
        return false;
    }
};

class XiaozuiClear: public TriggerSkill {
public:
    XiaozuiClear(): TriggerSkill("#xiaozui-clear") {
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && !target->hasSkill("xiaozui") && target->getPile("xiaozuipile").length() > 0;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &) const{
        player->clearOnePrivatePile("xiaozuipile");
        return false;
    }
};

AnxuCard::AnxuCard(){
    once = true;
}

bool AnxuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;
    if(targets.isEmpty())
        return true;
    else if(targets.length() == 1)
        return to_select->getHandcardNum() != targets.first()->getHandcardNum();
    else
        return false;
}

bool AnxuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void AnxuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> selecteds = targets;
    ServerPlayer *from = selecteds.first()->getHandcardNum() < selecteds.last()->getHandcardNum() ? selecteds.takeFirst() : selecteds.takeLast();
    ServerPlayer *to = selecteds.takeFirst();
    int id = room->askForCardChosen(from, to, "h", "anxu");
    const Card *cd = Sanguosha->getCard(id);
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName());
    room->obtainCard(from, cd, reason);
    if(cd->getSuit() != Card::Spade){
        source->drawCards(1);
    }
}

class Anxu: public ZeroCardViewAsSkill{
public:
    Anxu():ZeroCardViewAsSkill("anxu"){
    }

    virtual const Card *viewAs() const{
        return new AnxuCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("AnxuCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }
};

class Zhuiyi: public TriggerSkill {
public:
    Zhuiyi(): TriggerSkill("zhuiyi") {
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player)
            return false;
        QList<ServerPlayer *> targets = (death.damage && death.damage->from) ? room->getOtherPlayers(death.damage->from) :
                                                                               room->getAlivePlayers();

        QVariant data_for_ai = QVariant::fromValue(death.damage);
        if (targets.isEmpty() || !player->askForSkillInvoke(objectName(), data_for_ai))
            return false;

        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
        
        target->drawCards(3);
        RecoverStruct recover;
        recover.who = player;
        recover.recover = 1;
        room->recover(target, recover, true);
        return false;
    }
};

class Xuanren : public OneCardViewAsSkill{
public:
    Xuanren():OneCardViewAsSkill("xuanren"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player, NULL);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const Card *to_select) const{
        if (to_select->getTypeId() != Card::TypeEquip)
            return false;

        if (Self->getWeapon() && to_select->getEffectiveId() == Self->getWeapon()->getId() && to_select->objectName() == "Crossbow")
            return Self->canSlashWithoutCrossbow();
        else
            return true;
    }

    const Card *viewAs(const Card *originalCard) const{
        Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->addSubcard(originalCard);
        slash->setSkillName(objectName());
        return slash;
    }
};

class XuanrenTargetMod: public TargetModSkill {
public:
    XuanrenTargetMod(): TargetModSkill("#xuanren-target") {
        frequency = NotFrequent;
    }

    virtual int getDistanceLimit(const Player *, const Card *card) const{
        if (card->getSkillName() == "xuanren")
            return 1000;
        else
            return 0;
    }
};

class Jieyou : public TriggerSkill{
public:
    Jieyou():TriggerSkill("jieyou"){
        events << Dying << DamageCaused << CardFinished << CardUsed;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *current = room->getCurrent();
        if (!current || current->isDead())
            return false;
        if (triggerEvent == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from != player || !player->hasFlag("jieyouUsed"))
                return false;
            if (use.card->isKindOf("Slash")) {
                room->setPlayerFlag(player, "-jieyouUsed");
                room->setCardFlag(use.card, "jieyou-slash");
            }
        } else if (triggerEvent == Dying && room->askForSkillInvoke(player, objectName(), data)) {
            DyingStruct dying = data.value<DyingStruct>();

            forever {
                if (player->hasFlag("jieyou_failed")) {
                    room->setPlayerFlag(player, "-jieyou_failed");
                    break;
                }

                if (dying.who->getHp() > 0 || player->isNude() || !current
                    || current->isDead())
                    break;

                room->setPlayerFlag(player, "jieyouUsed");
                room->setTag("JieyouTarget", data);
                bool use_slash = room->askForUseSlashTo(player, current, "jieyou-slash:" + current->objectName(), true, false);
                if (!use_slash) {
                    room->setPlayerFlag(player, "-jieyouUsed");
                    room->removeTag("JieyouTarget");
                    break;
                }
            }

            return dying.who->getHp() > 0;
        } else if(triggerEvent == DamageCaused) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from && damage.from == player && damage.card && damage.card->isKindOf("Slash")
                && damage.card->hasFlag("jieyou-slash"))
            {
                LogMessage log2;
                log2.type = "#JieyouPrevent";
                log2.from = player;
                log2.to << damage.to;
                room->sendLog(log2);

                DyingStruct dying = room->getTag("JieyouTarget").value<DyingStruct>();

                ServerPlayer *target = dying.who;
                if (target && target->getHp() > 0) {
                    LogMessage log;
                    log.type = "#JieyouNull1";
                    log.from = dying.who;
                    room->sendLog(log);
                } else if (target && target->isDead()) {
                    LogMessage log;
                    log.type = "#JieyouNull2";
                    log.from = dying.who;
                    log.to << player;
                    room->sendLog(log);
                } else {
                    Peach *peach = new Peach(Card::NoSuitNoColor, 0);
                    peach->setSkillName(objectName());
                    CardUseStruct use;
                    use.card = peach;
                    use.from = player;
                    use.to << target;

                    room->setCardFlag(damage.card, "jieyou_success");
                    room->useCard(use);
                }
                return true;
            }
            return false;
        } else if (triggerEvent == CardFinished && !room->getTag("JieyouTarget").isNull()) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from == player && use.card->hasFlag("jieyou-slash")) {
                if (!use.card->hasFlag("jieyou_success"))
                    room->setPlayerFlag(player, "jieyou_failed");
                room->removeTag("JieyouTarget");
            }
        }

        return false;
    }
};

FenxunCard::FenxunCard() {
}

bool FenxunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FenxunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.from->tag["FenxunTarget"] = QVariant::fromValue(effect.to);
    room->setFixedDistance(effect.from, effect.to, 1);
}

class FenxunViewAsSkill: public OneCardViewAsSkill {
public:
    FenxunViewAsSkill(): OneCardViewAsSkill("fenxun") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isNude() && !player->hasUsed("FenxunCard");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !Self->isHuyin(to_select);
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        FenxunCard *first = new FenxunCard;
        first->addSubcard(originalcard->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Fenxun: public TriggerSkill {
public:
    Fenxun(): TriggerSkill("fenxun") {
        events << EventPhaseChanging << Death << EventLoseSkill;
        view_as_skill = new FenxunViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->tag["FenxunTarget"].value<PlayerStar>() != NULL;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *dingfeng, QVariant &data) const{
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.who != dingfeng || change.to != Player::NotActive)
                return false;
        }
        if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != dingfeng)
                return false;
        }
        ServerPlayer *target = dingfeng->tag["FenxunTarget"].value<PlayerStar>();

        if (target) {
            room->setFixedDistance(dingfeng, target, -1);
            dingfeng->tag.remove("FenxunTarget");
        }
        return false;
    }
};

class Qianbian: public TriggerSkill {
public:
    Qianbian(): TriggerSkill("qianbian") {
        events << CardsMoveOneTime;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *lingtong, QVariant &data) const{
        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
        if (move->from != lingtong)
            return false;

        if (move->from_places.contains(Player::PlaceEquip)) {
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(lingtong);
            foreach (ServerPlayer *player, other_players)
                if (!player->isNude()) {
                    can_invoke = true;
                    break;
                }

            if (!can_invoke)
                return false;

            if (lingtong->askForSkillInvoke(objectName())) {
                room->broadcastSkillInvoke(objectName());
                QList<ServerPlayer *> targets;
                foreach (ServerPlayer *target, room->getOtherPlayers(lingtong))
                    if (!target->isNude())
                        targets << target;

                ServerPlayer *first = room->askForPlayerChosen(lingtong, targets, objectName());
                ServerPlayer *second = NULL;
                int first_id = -1;
                int second_id = -1;
                if (first != NULL) {
                    first_id = room->askForCardChosen(lingtong, first, "he", objectName());
                    room->throwCard(first_id, first, lingtong);
                    if (first->isNude())
                        targets.removeOne(first);
                }
                can_invoke = false;
                QList<ServerPlayer *> other_players = room->getOtherPlayers(lingtong);
                foreach (ServerPlayer *player, other_players)
                    if (!player->isNude()) {
                        can_invoke = true;
                        break;
                    }

                if (can_invoke)
                    second = room->askForPlayerChosen(lingtong, targets, objectName());
                if (second != NULL) {
                    second_id = room->askForCardChosen(lingtong, second, "he", objectName());
                    room->throwCard(second_id, second, lingtong);
                }

                if (first_id != -1 && Sanguosha->getEngineCard(first_id)->isKindOf("BasicCard"))
                    lingtong->drawCards(1);
                if (second_id != -1 && Sanguosha->getEngineCard(second_id)->isKindOf("BasicCard"))
                    lingtong->drawCards(1);
            }
        }

        return false;
    }
};

MengjingCard::MengjingCard() {
    target_fixed = true;
}

void MengjingCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->setPlayerFlag(source, "InfinityAttackRange");
    const Card *cd = Sanguosha->getCard(subcards.first());
    if (cd->isKindOf("EquipCard")) {
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(source))
            if (!p->isNude()) targets << p;
        if (!targets.isEmpty() && source->askForSkillInvoke("mengjing")) {
            ServerPlayer *to_discard = room->askForPlayerChosen(source, targets, "mengjing");
            room->throwCard(room->askForCardChosen(source, to_discard, "he", "mengjing"), to_discard, source);
        }
    }
}

class Mengjing: public OneCardViewAsSkill {
public:
    Mengjing(): OneCardViewAsSkill("mengjing") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MengjingCard");
    }

    virtual bool viewFilter(const Card *) const{
        return true;
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        MengjingCard *card = new MengjingCard;
        card->addSubcard(originalcard->getId());
        card->setSkillName(objectName());
        return card;
    }
};

ZhizhanCard::ZhizhanCard() {
    mute = true;
}

bool ZhizhanCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const{
    return targets.isEmpty();
}

void ZhizhanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    source->loseMark("@zhizhan");
    source->gainMark("@zhizhanused");
    ServerPlayer *target = targets.first();
    source->tag["ZhizhanTarget"] = QVariant::fromValue((PlayerStar)target);
    //room->broadcastInvoke("animate", "lightbox:$JiefanAnimate:2500");
    //room->getThread()->delay(2000);

    foreach (ServerPlayer *player, room->getAllPlayers()) {
        if (player->isAlive() && player->inMyAttackRange(target))
            room->cardEffect(this, source, player);
    }
    source->tag.remove("ZhizhanTarget");
}

void ZhizhanCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    PlayerStar target = effect.from->tag["ZhizhanTarget"].value<PlayerStar>();
    QVariant data = effect.from->tag["ZhizhanTarget"];
    if (target && !room->askForCard(effect.to, ".Weapon", "@zhizhan-discard", data))
        target->drawCards(1);
}

class Zhizhan: public ZeroCardViewAsSkill {
public:
    Zhizhan(): ZeroCardViewAsSkill("zhizhan") {
        frequency = Limited;
    }

    virtual const Card *viewAs() const{
        return new ZhizhanCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@zhizhan") >= 1;
    }
};

static bool CompareBySuit(int card1, int card2) {
    const Card *c1 = Sanguosha->getCard(card1);
    const Card *c2 = Sanguosha->getCard(card2);

    int a = static_cast<int>(c1->getSuit());
    int b = static_cast<int>(c2->getSuit());

    return a < b;
}

class Lvejue: public TriggerSkill {
public:
    Lvejue(): TriggerSkill("lvejue") {
        events << EventPhaseStart;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const {
        if (data.value<PlayerStar>() != player || player->getPhase() != Player::Draw)
            return false;

        if (!player->askForSkillInvoke(objectName()))
            return false;

        room->broadcastSkillInvoke(objectName());

        QList<int> card_ids = room->getNCards(5);
        qSort(card_ids.begin(), card_ids.end(), CompareBySuit);
        room->fillAG(card_ids);

        while (!card_ids.isEmpty()) {
            int card_id = room->askForAG(player, card_ids, true, objectName());
            card_ids.removeOne(card_id);
            // throw the rest cards that matches the same suit
            const Card *card = Sanguosha->getCard(card_id);
            Card::Suit suit = card->getSuit();

            room->takeAG(player, card_id);

            QMutableListIterator<int> itor(card_ids);
            while (itor.hasNext()) {
                const Card *c = Sanguosha->getCard(itor.next());
                if (c->getSuit() == suit) {
                    itor.remove();
                    room->takeAG(NULL, c->getId());
                }
            }
        }

        room->broadcastInvoke("clearAG");

        return true;
    }
};

LingshiCard::LingshiCard(){
    once = true;
}

void LingshiCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->doLingshi(effect.from, effect.to);
}

class Lingshi: public ZeroCardViewAsSkill{
public:
    Lingshi():ZeroCardViewAsSkill("lingshi"){
        default_choice = "discard";
    }

    virtual const Card *viewAs() const{
        return new LingshiCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LingshiCard");
    }
};

class Longxi: public TriggerSkill {
public:
    Longxi(): TriggerSkill("longxi") {
        events << CardsMoveOneTime << EventPhaseStart << EventPhaseEnd;
        default_choice = "down";
    }

    void perform(ServerPlayer *player) const{
        Room *room = player->getRoom();
        QString result = room->askForChoice(player, objectName(), "up+down");
        QList<ServerPlayer *> all_players = room->getAllPlayers();
        if (result == "up") {
            room->broadcastSkillInvoke(objectName(), 2);
            foreach (ServerPlayer *player, all_players) {
                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
            }
        } else if (result == "down") {
            foreach (ServerPlayer *player, all_players) {
                room->loseHp(player);
            }

            room->broadcastSkillInvoke(objectName(), 1);
        }
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player->getPhase() != Player::Discard)
            return false;

        if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if (move->from == player && move->to_place == Player::DiscardPile
                && (move->reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
                player->setMark("longxi", player->getMark("longxi") + move->card_ids.size());
            }
        } else if (triggerEvent == EventPhaseStart && data.value<PlayerStar>() == player) {
            player->setMark("longxi", 0);
        } else if (triggerEvent == EventPhaseEnd && data.value<PlayerStar>() == player
                   && player->getMark("longxi") >= 2 && player->askForSkillInvoke(objectName()))
            perform(player);

        return false;
    }
};

void YeyanCard::damage(ServerPlayer *player, ServerPlayer *target, int point) const{
    DamageStruct damage;

    damage.card = NULL;
    damage.from = player;
    damage.to = target;
    damage.damage = point;
    damage.nature = DamageStruct::Fire;

    player->getRoom()->damage(damage);
}

GreatYeyanCard::GreatYeyanCard() {
    mute = true;
    m_skillName = "yeyan";
}

bool GreatYeyanCard::targetFilter(const QList<const Player *> &, const Player *, const Player *) const{
    Q_ASSERT(false);
    return false;
}

bool GreatYeyanCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    if (subcards.length() != 4) return false;
    QList<Card::Suit> allsuits;
    foreach (int cardId, subcards) {
        const Card *card = Sanguosha->getCard(cardId);
        if (allsuits.contains(card->getSuit())) return false;
        allsuits.append(card->getSuit());
    }
    
    //We can only assign 2 damage to one player
    //If we select only one target only once, we assign 3 damage to the target
    if (targets.toSet().size() == 1)
        return true;
    else if (targets.toSet().size() == 2)
        return targets.size() == 3;
    return false;
}

bool GreatYeyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select,
                                  const Player *, int &maxVotes) const{
    int i = 0;
    foreach(const Player *player, targets)
        if (player == to_select) i++;
    maxVotes = qMax(3 - targets.size(),0) + i;
    return maxVotes > 0;
}

void GreatYeyanCard::use(Room *room, ServerPlayer *player, QList<ServerPlayer *> &targets) const{
    int criticaltarget = 0;
    int totalvictim = 0;
    QMap<ServerPlayer *, int> map;

    foreach(ServerPlayer *sp, targets)
        map[sp]++;

    if (targets.size() == 1)
        map[targets.first()] += 2;

    foreach(ServerPlayer *sp, map.keys()){
        if(map[sp] > 1)
            criticaltarget++;
        totalvictim++;
    }
    if (criticaltarget > 0) {
        room->loseHp(player, 3);
        player->loseMark("@yeyan");
        player->gainMark("@yeyanused");
        if (totalvictim > 1) {
            //room->broadcastInvoke("animate", "lightbox:$YeyanAnimate");
            room->broadcastSkillInvoke("yeyan", 2);
        } else {
            //room->broadcastInvoke("animate", "lightbox:$YeyanAnimate");
            room->broadcastSkillInvoke("yeyan", 1);
        }
        QList<ServerPlayer *>targets = map.keys();
        if (targets.size() > 1)
            qSort(targets.begin(), targets.end(), ServerPlayer::CompareByActionOrder);
        foreach (ServerPlayer *sp, targets)
            damage(player, sp, map[sp]);
    }
}

SmallYeyanCard::SmallYeyanCard() {
    mute = true;
    m_skillName = "yeyan";
}

bool SmallYeyanCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return !targets.isEmpty();
}

bool SmallYeyanCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const{
    return targets.length() < 3;
}

void SmallYeyanCard::use(Room *room, ServerPlayer *player, QList<ServerPlayer *> &targets) const{
    //room->broadcastInvoke("animate", "lightbox:$YeyanAnimate");
    room->broadcastSkillInvoke("yeyan", 3);
    player->loseMark("@yeyan");
    player->gainMark("@yeyanused");
    Card::use(room, player, targets);
}

void SmallYeyanCard::onEffect(const CardEffectStruct &effect) const{
    damage(effect.from, effect.to, 1);
}

class Yeyan: public ViewAsSkill {
public:
    Yeyan(): ViewAsSkill("yeyan") {
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@yeyan") > 0;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (selected.length() >= 4)
            return false;

        if (to_select->isEquipped())
            return false;

        if (Self->isHuyin(to_select))
            return false;

        foreach (const Card *item, selected) {
            if (to_select->getSuit() == item->getSuit())
                return false;
        }

        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length()  == 0) 
            return new SmallYeyanCard;
        if (cards.length() != 4)
            return NULL;

        GreatYeyanCard *card = new GreatYeyanCard;
        card->addSubcards(cards);

        return card;
    }
};

void StandardPackage::addSnowGenerals(){
    General *snow001 = new General(this, "snow001$", "wu");
    snow001->addSkill(new Zhiheng);
    snow001->addSkill(new Jiyuan);

    General *snow002 = new General(this, "snow002", "wu");
    snow002->addSkill(new Kuipo);

    General *snow003 = new General(this, "snow003", "wu");
    snow003->addSkill(new Biju);
    snow003->addSkill(new Pojian);
    snow003->addRelateSkill("qinghua");

    General *snow004 = new General(this, "snow004", "wu");
    snow004->addSkill(new Kurou);
    snow004->addSkill(new Zaiqi);

    General *snow005 = new General(this, "snow005", "wu", 3);
    snow005->addSkill(new Guideng);
    snow005->addSkill(new Chenhong);

    General *snow006 = new General(this, "snow006", "wu", 3, false);
    snow006->addSkill(new Wanmei);
    snow006->addSkill(new Xuanhuo);

    General *snow007 = new General(this, "snow007", "wu", 3);
    snow007->addSkill(new Bujie);
    snow007->addSkill(new Yuanhe);

    General *snow008 = new General(this, "snow008", "wu", 3, false);
    snow008->addSkill(new Yulu);
    snow008->addSkill(new Cuimeng);

    General *snow009 = new General(this, "snow009", "wu");
    snow009->addSkill(new Liangban);

    General *snow010 = new General(this, "snow010", "wu", 3);
    snow010->addSkill(new Jilian);
    snow010->addSkill(new JilianViewAsSkill);
    snow010->addSkill(new JilianGive);
    snow010->addSkill(new Qixi);
    related_skills.insertMulti("jilian", "#jilian");
    related_skills.insertMulti("jilian", "#jilian-give");

    General *snow011 = new General(this, "snow011", "wu", 3, false);
    snow011->addSkill(new Zhihui);
    snow011->addSkill(new Chiqiu);

    General *snow012 = new General(this, "snow012", "wu");
    snow012->addSkill(new Jianmie);
    snow012->addSkill(new JianmieTargetMod);
    related_skills.insertMulti("jianmie", "#jianmie-target");

    General *snow013 = new General(this, "snow013", "wu");
    snow013->addSkill(new Susheng);
    snow013->addSkill(new SushengRemove);
    snow013->addSkill(new Eli);
    related_skills.insertMulti("susheng", "#susheng-remove");

    General *snow014 = new General(this, "snow014$", "wu");
    snow014->addSkill(new Heyi);
    snow014->addSkill(new Chizhu);
    snow014->addSkill(new Biansheng);

    General *snow015 = new General(this, "snow015", "wu", 3);
    snow015->addSkill(new Jiban);
    snow015->addSkill(new Jizhou);
    snow015->addSkill(new JizhouGet);
    related_skills.insertMulti("jizhou", "#jizhou-get");
    
    General *snow016 = new General(this, "snow016", "wu");
    snow016->addSkill(new Leilan);

    General *snow017 = new General(this, "snow017", "wu", 3, false);
    snow017->addSkill(new Yuanfa);
    snow017->addSkill(new Guanju);

    General *snow018 = new General(this, "snow018", "wu");
    snow018->addSkill(new Zhongqu);

    General *snow019 = new General(this, "snow019", "wu");
    snow019->addSkill(new Lingpao);
    snow019->addSkill(new LingpaoTargetMod);
    snow019->addSkill(new Xiaozui);
    snow019->addSkill(new XiaozuiClear);
    related_skills.insertMulti("lingpao", "#lingpao-target");
    related_skills.insertMulti("xiaozui", "#xiaozui-clear");

    General *snow020 = new General(this, "snow020", "wu", 3, false);
    snow020->addSkill(new Anxu);
    snow020->addSkill(new Zhuiyi);

    General *snow021 = new General(this, "snow021", "wu");
    snow021->addSkill(new Xuanren);
    snow021->addSkill(new XuanrenTargetMod);
    related_skills.insertMulti("xuanren", "#xuanren-target");
    snow021->addSkill(new Jieyou);

    General *snow022 = new General(this, "snow022", "wu");
    snow022->addSkill(new Skill("xindu"));
    snow022->addSkill(new Fenxun);

    General *snow023 = new General(this, "snow023", "wu");
    snow023->addSkill(new Qianbian);

    General *snow024 = new General(this, "snow024", "wu");
    snow024->addSkill(new Mengjing);
    snow024->addSkill(new Zhizhan);
    snow024->addSkill(new MarkAssignSkill("@zhizhan", 1));
    related_skills.insertMulti("zhizhan", "#@zhizhan-1");

    General *snow029 = new General(this, "snow029", "wu", 3);
    snow029->addSkill(new Lvejue);
    snow029->addSkill(new Lingshi);
    
    General *snow030 = new General(this, "snow030", "wu");
    snow030->addSkill(new Longxi);
    snow030->addSkill(new MarkAssignSkill("@yeyan", 1));
    snow030->addSkill(new Yeyan);
    related_skills.insertMulti("yeyan", "#@yeyan-1");

    addMetaObject<ZhihengCard>();
    addMetaObject<QinghuaCard>();
    addMetaObject<KurouCard>();
    addMetaObject<GuidengCard>();
    addMetaObject<YuanheCard>();
    addMetaObject<YuluCard>();
    addMetaObject<LiangbanCard>();
    addMetaObject<JilianCard>();
    addMetaObject<QixiCard>();
    addMetaObject<ZhihuiCard>();
    addMetaObject<JianmieCard>();
    addMetaObject<BianshengCard>();
    addMetaObject<JibanCard>();
    addMetaObject<GuanjuCard>();
    addMetaObject<XiaozuiCard>();
    addMetaObject<AnxuCard>();
    addMetaObject<FenxunCard>();
    addMetaObject<MengjingCard>();
    addMetaObject<ZhizhanCard>();
    addMetaObject<LingshiCard>();
    addMetaObject<YeyanCard>();
    addMetaObject<GreatYeyanCard>();
    addMetaObject<SmallYeyanCard>();

    skills << new Qinghua << new BianshengPindian;
}
