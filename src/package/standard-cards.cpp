#include "standard.h"
#include "standard-equips.h"
#include "maneuvering.h"
#include "general.h"
#include "engine.h"
#include "client.h"
#include "room.h"
#include "carditem.h"

Slash::Slash(Suit suit, int number): BasicCard(suit, number)
{
    setObjectName("slash");
    nature = DamageStruct::Normal;
}

DamageStruct::Nature Slash::getNature() const{
    return nature;
}

void Slash::setNature(DamageStruct::Nature nature){
    this->nature = nature;
}

bool Slash::IsAvailable(const Player *player, const Card *slash){
    if (!slash) {
        slash = new Slash(Card::NoSuitNoColor, 0);
    }
    
    if (player->isCardLimited(slash, Card::MethodUse))
        return false;

    return (player->hasWeapon("Crossbow") || player->canSlashWithoutCrossbow());
}

bool Slash::isAvailable(const Player *player) const{
    return (IsAvailable(player, this) || player->canUseExtraSlash(this)) && BasicCard::isAvailable(player);
}

QString Slash::getSubtype() const{
    return "attack_card";
}

void Slash::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    ServerPlayer *player = use.from;

    if (player->hasFlag("slashTargetFix")) {
        room->setPlayerFlag(player, "-slashTargetFix");
        room->setPlayerFlag(player, "-slashTargetFixToOne");
        foreach (ServerPlayer *target, room->getAlivePlayers())
            if (target->hasFlag("SlashAssignee"))
                room->setPlayerFlag(target, "-SlashAssignee");
        if (player->hasFlag("slashNoDistanceLimit"))
            room->setPlayerFlag(player, "-slashNoDistanceLimit");
        if (player->hasFlag("slashDisableExtraTarget"))
            room->setPlayerFlag(player, "-slashDisableExtraTarget");
    }

    if (objectName() == "slash") {
        bool has_changed = false;
        QString skill_name = getSkillName();
        if (!skill_name.isEmpty()) {
            const Skill *skill = Sanguosha->getSkill(skill_name);
            if (skill && !skill->inherits("FilterSkill") && skill->objectName() != "guhuo")
                has_changed = true;
        }
        if (!has_changed || subcardsLength() == 0) {
            QVariant data = QVariant::fromValue(use);
            if (player->hasSkill("lingpao"))
                if (room->askForSkillInvoke(player, "lingpao", data)) {
                    FireSlash *fire_slash = new FireSlash(getSuit(), getNumber());
                    if (subcardsLength() > 0)
                        fire_slash->addSubcard(this);
                    fire_slash->setSkillName("lingpao");
                    use.card = fire_slash;
                }
            if (player->hasSkill("Fan") && !use.card->isKindOf("FireSlash")) {
                if (room->askForSkillInvoke(player, "Fan", data)) {
                    FireSlash *fire_slash = new FireSlash(getSuit(), getNumber());
                    if (subcardsLength() > 0)
                        fire_slash->addSubcard(this);
                    fire_slash->setSkillName("Fan");
                    use.card = fire_slash;
                }
            }
        }
    }
    if ((use.card->isVirtualCard() && use.card->subcardsLength() == 0) || (getSkillName() == "guhuo" && use.card != this)) {
        QList<ServerPlayer *> targets_ts;
        while (true) {
            QList<const Player *> targets_const;
            foreach (ServerPlayer *p, use.to)
                targets_const << qobject_cast<const Player *>(p);
            foreach (ServerPlayer *p, room->getAlivePlayers())
                if (!use.to.contains(p) && use.card->targetFilter(targets_const, p, use.from))
                    targets_ts << p;
            if (targets_ts.isEmpty())
                break;
            else if (room->askForSkillInvoke(player, "slash_extra_targets", "yes")) {
                ServerPlayer *extra_target = room->askForPlayerChosen(player, targets_ts, "slash_extra_targets");
                use.to.append(extra_target);
                qSort(use.to.begin(), use.to.end(), ServerPlayer::CompareByActionOrder);
            } else
                break;
            targets_ts.clear();
            targets_const.clear();
        }
    }

    if (player->getPhase() == Player::Play && player->hasFlag("MoreSlashInOneTurn")) {
        if (player->hasSkill("hupao"))
            room->broadcastSkillInvoke("hupao");
        else if (player->hasSkill("canyue"))
            room->broadcastSkillInvoke("canyue");
    }
    if (use.to.size() > 1 && player->hasSkill("shenji"))
        room->broadcastSkillInvoke("shenji");
    else if (use.to.size() > 1 && player->hasSkill("lingpao") && use.card->isKindOf("FireSlash") && use.card->getSkillName() != "lingpao")
        room->broadcastSkillInvoke("lingpao", 1);
    else if (use.to.size() > 1 && player->hasSkill("xindu"))
        room->broadcastSkillInvoke("xindu");

    if (use.from->hasFlag("BladeUse")) {
        room->setEmotion(player, "weapon/blade");

        LogMessage log;
        log.type = "#BladeUse";
        log.from = use.from;
        log.to << use.to;
        room->sendLog(log);
    } else if (isVirtualCard() && use.card->getSkillName() == "Spear")
        room->setEmotion(player,"weapon/spear");
    else if (use.to.size() > 1 && player->hasWeapon("Halberd") && player->isLastHandCard(this))
        room->setEmotion(player,"weapon/halberd");
    else if (isVirtualCard() && use.card->getSkillName() == "Fan")
        room->setEmotion(player,"weapon/fan");
    if (player->getPhase() == Player::Play
        && player->hasFlag("MoreSlashInOneTurn")
        && player->hasWeapon("Crossbow")
        && !player->hasSkill("hupao")
        && !player->hasSkill("canyue"))
        room->setEmotion(player,"weapon/crossbow");
    if (use.card->isKindOf("ThunderSlash"))
        room->setEmotion(player, "thunder_slash");
    else if (use.card->isKindOf("FireSlash"))
        room->setEmotion(player, "fire_slash");
    else if (use.card->isRed())
        room->setEmotion(player, "slash_red");
    else if (use.card->isBlack())
        room->setEmotion(player, "slash_black");
    else
        room->setEmotion(player, "killer");

    BasicCard::onUse(room, use);
}

void Slash::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    foreach (ServerPlayer *target, targets)
        if (target->hasFlag("CollateralTarget"))
            room->setPlayerFlag(target, "-CollateralTarget");

    BasicCard::use(room, source, targets);

    if (hasFlag("drank")) {
        LogMessage log;
        log.type = "#UnsetDrank";
        log.from = source;
        room->sendLog(log);
    }
}

void Slash::onEffect(const CardEffectStruct &card_effect) const{
    Room *room = card_effect.from->getRoom();
    if (card_effect.from->hasFlag("drank")) {
        room->setCardFlag(this, "drank");
        room->setPlayerFlag(card_effect.from, "-drank");
    }

    SlashEffectStruct effect;
    effect.from = card_effect.from;
    if (hasFlag("isFireSlash") && nature == DamageStruct::Normal)
        effect.nature = DamageStruct::Fire;
    else
        effect.nature = nature;
    effect.slash = this;

    effect.to = card_effect.to;
    effect.drank = hasFlag("drank");

    room->slashEffect(effect);
}

bool Slash::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return !targets.isEmpty();
}

bool Slash::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{    
    int slash_targets = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    bool distance_limit = ((1 + Sanguosha->correctCardTarget(TargetModSkill::DistanceLimit, Self, this)) < 500);
    if (Self->hasFlag("slashNoDistanceLimit"))
        distance_limit = false;

    int rangefix = 0;
    if (Self->getWeapon() && subcards.contains(Self->getWeapon()->getId())) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        rangefix += weapon->getRange() - 1;
    }

    if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId()))
        rangefix += 1;

    if (Sanguosha->correctCardTarget(TargetModSkill::DistanceLimit, Self, this) != 0)
        rangefix -= Sanguosha->correctCardTarget(TargetModSkill::DistanceLimit, Self, this);

    if (Self->hasFlag("slashTargetFix")) {
        if (targets.isEmpty())
            return to_select->hasFlag("SlashAssignee") && Self->canSlash(to_select, this, distance_limit, rangefix);
        else {
            if (Self->hasFlag("slashDisableExtraTarget")) return false;
            bool canSelect = false;
            foreach (const Player *p, targets) {
                if(p->hasFlag("SlashAssignee")){
                    canSelect = true;
                    break;
                }
            }
            if(!canSelect) return false;
        }
    }

    if (targets.length() >= slash_targets) {
        if (Self->hasSkill("xindu") && targets.length() == slash_targets) {
            bool hasExtraTarget = false;
            foreach (const Player *p, targets)
                if (Self->distanceTo(p) == 1) {
                    hasExtraTarget = true;
                    break;
                }
            if (hasExtraTarget)
                return Self->canSlash(to_select, this, distance_limit, rangefix);
            else
                return Self->canSlash(to_select, this) && Self->distanceTo(to_select) == 1;
        } else
            return false;
    }

    return Self->canSlash(to_select, this, distance_limit, rangefix);

    /*if (isKindOf("ThunderSlash") && Self->hasSkill("thyuchang"))
        distance_limit = false;

    if(getSuit() == Card::Heart && Self->hasSkill("zhenhong"))
        distance_limit = false;*/

    /*if (Self->hasSkill("xiagong") && !Self->getWeapon())
        return Self->canSlash(to_select, this, false) && Self->distanceTo(to_select) <= 2;
    else
        return Self->canSlash(to_select, this, distance_limit, rangefix);*/
}

Jink::Jink(Suit suit, int number):BasicCard(suit, number){
    setObjectName("jink");

    target_fixed = true;
}

QString Jink::getSubtype() const{
    return "defense_card";
}

bool Jink::isAvailable(const Player *) const{
    return false;
}

Peach::Peach(Suit suit, int number):BasicCard(suit, number){
    setObjectName("peach");
    target_fixed = true;
}

QString Peach::getSubtype() const{
    return "recover_card";
}

void Peach::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    BasicCard::use(room, source, targets);
    if(targets.isEmpty())
        room->cardEffect(this, source, source);
}

void Peach::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    // do animation
    room->broadcastInvoke("animate", QString("peach:%1:%2")
        .arg(effect.from->objectName())
        .arg(effect.to->objectName()));

    // recover hp
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    room->recover(effect.to, recover);
}

bool Peach::isAvailable(const Player *player) const{
    const Player *target = NULL;
    foreach (const Player *p, player->getSiblings()) {
        if (p->hasFlag("dying")) {
            target = p;
            break;
        }
    }
    if (target) {
        if (player->isProhibited(target, this)) return false;
    } else {
        if (player->isProhibited(player, this)) return false;
    }

    return player->isWounded() && BasicCard::isAvailable(player);
}

Crossbow::Crossbow(Suit suit, int number)
    :Weapon(suit, number, 1)
{
    setObjectName("Crossbow");
}

class DoubleSwordSkill: public WeaponSkill{
public:
    DoubleSwordSkill():WeaponSkill("DoubleSword"){
        events << TargetConfirmed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return WeaponSkill::triggerable(target) && !target->isSexLess();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.from != player)
            return false;

        foreach(ServerPlayer *to, use.to){
            if(use.from->isMale() != to->isMale()
                && !to->isSexLess()
                && use.card->isKindOf("Slash")){
                if(use.from->askForSkillInvoke(objectName())){
                    to->getRoom()->setEmotion(use.from,"weapon/double_sword");
                    bool draw_card = false;

                    if(to->isKongcheng())
                        draw_card = true;
                    else{
                        QString prompt = "double-sword-card:" + use.from->getGeneralName();
                        const Card *card = room->askForCard(to, ".", prompt);
                        if(!card)
                            draw_card = true;
                    }
                    if(draw_card)
                       use.from->drawCards(1);
                }
            }
        }

        return false;
    }
};

DoubleSword::DoubleSword(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("DoubleSword");
}

class QinggangSwordSkill: public WeaponSkill{
public:
    QinggangSwordSkill():WeaponSkill("QinggangSword"){
        events << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (WeaponSkill::triggerable(use.from) && use.from == player && use.card->isKindOf("Slash")) {
            bool do_anim = false;
            foreach (ServerPlayer *p, use.to.toSet()) {
                if (p->getMark("Equips_of_Others_Nullified_to_You") == 0) {
                    p->addMark("Qinggang_Armor_Nullified");
                    if (p->getArmor() || p->hasSkill("shengtang"))  do_anim = true;
                }
            }
            if (do_anim)
                room->setEmotion(use.from, "weapon/qinggang_sword");
        }

        return false;
    }
};

QinggangSword::QinggangSword(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("QinggangSword");
}

class BladeSkill : public WeaponSkill{
public:
    BladeSkill():WeaponSkill("Blade"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if (!effect.to->isAlive() || effect.to->getMark("Equips_of_Others_Nullified_to_You") > 0)
            return false;
        if (!effect.from->canSlash(effect.to, NULL, false))
            return false;

        int weapon_id = player->getWeapon()->getId();
        room->setCardFlag(weapon_id, "using");
        room->setPlayerFlag(effect.from, "BladeUse");
        room->askForUseSlashTo(effect.from, effect.to, QString("blade-slash:%1").arg(effect.to->objectName()), false, false, true);
        room->setPlayerFlag(effect.from, "-BladeUse");
        room->setCardFlag(weapon_id, "-using");

        return false;
    }
};

Blade::Blade(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("Blade");
}

class SpearSkill: public ViewAsSkill{
public:
    SpearSkill():ViewAsSkill("Spear"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        Slash *slash = new Slash(Card::NoSuitNoColor, 0);
        slash->deleteLater();
        return !player->isCardLimited(slash, Card::MethodUse, true) && player->getMark("Equips_Nullified_to_Yourself") == 0;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash" && player->getMark("Equips_Nullified_to_Yourself") == 0;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() < 2 && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        const Card *first = cards[0];
        const Card *second = cards[1];

        Card::Suit suit = Card::NoSuitNoColor;
        if(first->isBlack() && second->isBlack())
            suit = Card::NoSuitBlack;
        else if(first->isRed() && second->isRed())
            suit = Card::NoSuitRed;

        int number;
        if (first->getNumber() == second->getNumber())
            number = first->getNumber();
        else
            number = 0;

        Slash *slash = new Slash(suit, number);
        slash->setSkillName(objectName());
        slash->addSubcard(first);
        slash->addSubcard(second);

        return slash;
    }
};

Spear::Spear(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("Spear");
}

class AxeViewAsSkill: public ViewAsSkill{
public:
    AxeViewAsSkill():ViewAsSkill("Axe"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@Axe";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if(selected.length() >= 2)
            return false;

        if(to_select == Self->getWeapon())
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        DummyCard *card = new DummyCard;
        card->setSkillName(objectName());
        card->addSubcards(cards);
        return card;
    }
};

class AxeSkill: public WeaponSkill{
public:
    AxeSkill():WeaponSkill("Axe"){
        events << SlashMissed;
        view_as_skill = new AxeViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if (!effect.to->isAlive() || effect.to->getMark("Equips_of_Others_Nullified_to_You") > 0)
            return false;
        
        CardStar card = room->askForCard(player, "@Axe", "@axe:" + effect.to->objectName(), data, objectName());
        if(card){
            room->setEmotion(effect.to, "weapon/axe");

            LogMessage log;
            log.type = "#AxeSkill";
            log.from = player;
            log.to << effect.to;
            log.arg = objectName();
            room->sendLog(log);

            room->slashResult(effect, NULL);
        }

        return false;
    }
};

Axe::Axe(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("Axe");
}

class HalberdSkill: public TargetModSkill {
public:
    HalberdSkill(): TargetModSkill("Halberd") {
    }

    virtual int getExtraTargetNum(const Player *from, const Card *card) const{
        if (from->hasWeapon("Halberd") && from->isLastHandCard(card))
            return 2;
        else
            return 0;
    }
};

Halberd::Halberd(Suit suit, int number)
    :Weapon(suit, number, 4)
{
    setObjectName("Halberd");
}

class KylinBowSkill: public WeaponSkill{
public:
    KylinBowSkill():WeaponSkill("KylinBow"){
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        QStringList horses;
        if(damage.card && damage.card->isKindOf("Slash") && !damage.chain && !damage.transfer
           && damage.to->getMark("Equips_of_Others_Nullified_to_You") == 0){
            if(damage.to->getDefensiveHorse())
                horses << "dhorse";
            if(damage.to->getOffensiveHorse())
                horses << "ohorse";

            if(horses.isEmpty())
                return false;

            if (player == NULL) return false;
            if(!player->askForSkillInvoke(objectName(), data))
                return false;

            room->setEmotion(player, "weapon/kylin_bow");

            QString horse_type = room->askForChoice(player, objectName(), horses.join("+"));

            if(horse_type == "dhorse")
                room->throwCard(damage.to->getDefensiveHorse(), damage.to, damage.from);
            else if(horse_type == "ohorse")
                room->throwCard(damage.to->getOffensiveHorse(), damage.to, damage.from);
        }

        return false;
    }
};

KylinBow::KylinBow(Suit suit, int number)
    :Weapon(suit, number, 5)
{
    setObjectName("KylinBow");
}

class EightDiagramSkill: public ArmorSkill{
public:
    EightDiagramSkill():ArmorSkill("EightDiagram"){
        events << CardAsked;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        QString asked = data.toStringList().first();
        if(asked == "jink"){
            if(room->askForSkillInvoke(player, objectName())){
                room->setCardFlag(player->getArmor()->getId(), "using");
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;

                room->judge(judge);

                if (player->getArmor())
                    room->setCardFlag(player->getArmor()->getId(), "-using");

                if(judge.isGood()){
                    room->setEmotion(player, "armor/eight_diagram");
                    Jink *jink = new Jink(Card::NoSuitNoColor, 0);
                    jink->setSkillName(objectName());
                    room->provide(jink);

                    return true;
                }
            }
        }
        return false;
    }

    int getEffectIndex(const ServerPlayer *, const Card *) const{
        return -2;
    }
};

EightDiagram::EightDiagram(Suit suit, int number)
    :Armor(suit, number){
        setObjectName("EightDiagram");
}

AmazingGrace::AmazingGrace(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("amazing_grace");
    has_preact = true;
}

void AmazingGrace::doPreAction(Room *room, const CardUseStruct &card_use) const{
    //QList<ServerPlayer *> players = card_use.to.isEmpty() ? room->getAllPlayers() : card_use.to;
    QList<int> card_ids = room->getNCards(room->getAllPlayers().length());
    room->fillAG(card_ids);

    QVariantList ag_list;
    foreach(int card_id, card_ids){
        room->setCardFlag(card_id, "visible");
        ag_list << card_id;
    }
    room->setTag("AmazingGrace", ag_list);
}

void AmazingGrace::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    /*CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName());
    room->moveCardTo(this, source, NULL, Player::DiscardPile, reason);*/

    QList<ServerPlayer *> players = targets.isEmpty() ? room->getAllPlayers() : targets;
    GlobalEffect::use(room, source, players);
    QVariantList ag_list;
    ag_list = room->getTag("AmazingGrace").toList();

    // throw the rest cards
    foreach(QVariant card_id, ag_list){
        room->takeAG(NULL, card_id.toInt());
    }

    room->broadcastInvoke("clearAG");
}

void AmazingGrace::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QVariantList ag_list = room->getTag("AmazingGrace").toList();
    QList<int> card_ids;
    foreach(QVariant card_id, ag_list)
        card_ids << card_id.toInt();

    int card_id = room->askForAG(effect.to, card_ids, false, objectName());
    card_ids.removeOne(card_id);

    room->takeAG(effect.to, card_id);
    ag_list.removeOne(card_id);

    room->setTag("AmazingGrace", ag_list);
}

GodSalvation::GodSalvation(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("god_salvation");
}

bool GodSalvation::isCancelable(const CardEffectStruct &effect) const{
    return effect.to->isWounded() && TrickCard::isCancelable(effect);
}

void GodSalvation::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    room->recover(effect.to, recover);
}

SavageAssault::SavageAssault(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("savage_assault");
}

void SavageAssault::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *slash = room->askForCard(effect.to,
                                         "slash",
                                         "savage-assault-slash:"+ effect.from->objectName(),
                                         QVariant(),
                                         Card::MethodResponse,
                                         effect.from->isAlive() ? effect.from : NULL);
    if (slash) {
        if (slash->getSkillName() == "Spear") room->setEmotion(effect.to, "weapon/spear");
        room->setEmotion(effect.to, "killer");
    }
    else{
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.to = effect.to;
        damage.nature = DamageStruct::Normal;

        if(effect.from->isAlive())
            damage.from = effect.from;
        else
            damage.from = NULL;

        room->damage(damage);
    }
}

ArcheryAttack::ArcheryAttack(Card::Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("archery_attack");
}

void ArcheryAttack::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *jink = room->askForCard(effect.to,
                                        "jink",
                                        "archery-attack-jink:" + effect.from->objectName(),
                                        QVariant(),
                                        Card::MethodResponse,
                                        effect.from->isAlive() ? effect.from : NULL);        
    if(jink && jink->getSkillName() != "EightDiagram" && jink->getSkillName() != "shengtang")
        room->setEmotion(effect.to, "jink");
    if (!jink)
    {
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        if(effect.from->isAlive())
            damage.from = effect.from;
        else
            damage.from = NULL;
        damage.to = effect.to;
        damage.nature = DamageStruct::Normal;

        room->damage(damage);
    }
}

void SingleTargetTrick::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    if(!targets.isEmpty()){
        foreach(ServerPlayer *tmp, targets){
            effect.to = tmp;
            room->cardEffect(effect);
        }
    }
    else{
        effect.to = source;
        room->cardEffect(effect);
    }

    if(room->getCardPlace(this->getEffectiveId()) == Player::PlaceTable)
    {
        CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName());
        reason.m_skillName = this->getSkillName();
        if (targets.size() == 1) reason.m_targetId = targets.first()->objectName();
        room->moveCardTo(this, source, NULL, Player::DiscardPile, reason);
    }
}

Collateral::Collateral(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("collateral");
}

bool Collateral::isAvailable(const Player *player) const{
    bool canUse = false;
    foreach(const Player *p, player->getSiblings()){
        if(p->getWeapon() && p->isAlive()){
            canUse = true;
            break;
        }
    }
    return canUse && SingleTargetTrick::isAvailable(player);
}

bool Collateral::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;
    
    if (to_select == Self)
        return false;

    foreach(const Player *p, to_select->getSiblings())
        if (to_select->canSlash(p))
            return !Self->isProhibited(to_select, this) && to_select->getWeapon() != NULL;

    return false;
}

bool Collateral::doCollateral(Room *room, ServerPlayer *killer, ServerPlayer *victim, const QString &prompt) const{
    bool useSlash = false;
    if (killer->canSlash(victim, NULL, false))
    {
        room->setPlayerFlag(killer, "CollateralUsing");
        useSlash = room->askForUseSlashTo(killer, victim, prompt);
        room->setPlayerFlag(killer, "-CollateralUsing");
    }
    return useSlash;
}

void Collateral::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *source = effect.from;
    Room *room = source->getRoom();
    ServerPlayer *killer = effect.to;
    ServerPlayer *victim = room->getTag("collateralVictim").value<PlayerStar>();

    WrappedCard *weapon = killer->getWeapon();
    if(victim == NULL)
        return;

    QString prompt = QString("collateral-slash:%1:%2")
            .arg(source->objectName()).arg(victim->objectName());

    room->setPlayerFlag(source, "CollateralSource");
    if (victim->isDead()){
        if(source->isAlive() && killer->isAlive() && killer->getWeapon()){
            source->obtainCard(weapon);
        }
    }
    else if (source->isDead()){
        if (killer->isAlive()){
            doCollateral(room, killer, victim, prompt);
        }
    }
    else{
        if(killer->isDead()) ;
        else if(!killer->getWeapon()){
            doCollateral(room, killer, victim, prompt);
        }
        else{
            if(!doCollateral(room, killer, victim, prompt)){
                if(killer->getWeapon())
                    source->obtainCard(weapon);
            }
        }
    }
    
    room->setPlayerFlag(source, "-CollateralSource");
    room->removeTag("collateralVictim");
}

Nullification::Nullification(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("nullification");
}

void Nullification::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    // does nothing, just throw it
    CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName());
    room->moveCardTo(this, source, NULL, Player::DiscardPile, reason);
}

bool Nullification::isAvailable(const Player *) const{
    return false;
}

ExNihilo::ExNihilo(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("ex_nihilo");
    target_fixed = true;
}

bool ExNihilo::isAvailable(const Player *player) const{
    return !player->isProhibited(player, this);
}

void ExNihilo::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(2);
}

Duel::Duel(Suit suit, int number)
    :SingleTargetTrick(suit, number, true)
{
    setObjectName("duel");
}

bool Duel::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;
    if (to_select == Self)
        return false;

    return true;
}

void Duel::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *first = effect.to;
    ServerPlayer *second = effect.from;
    Room *room = first->getRoom();

    room->setEmotion(first, "duel");
    room->setEmotion(second, "duel");

    forever{
        if (!first->isAlive())
            break;
        if(second->getMark("WushuangTarget") > 0){
            const Card *slash = room->askForCard(first,
                                                 "slash",
                                                 "@wushuang-slash-1:" + second->objectName(),
                                                 QVariant(),
                                                 Card::MethodResponse,
                                                 second);
            if(slash == NULL)
                break;

            slash = room->askForCard(first, "slash",
                                     "@wushuang-slash-2:" + second->objectName(),
                                     QVariant(),
                                     Card::MethodResponse,
                                     second);
            if(slash == NULL)
                break;

        }else{
            const Card *slash = room->askForCard(first,
                                                 "slash",
                                                 "duel-slash:" + second->objectName(),
                                                 QVariant(),
                                                 Card::MethodResponse,
                                                 second);
            if(slash == NULL)
                break;
        }

        qSwap(first, second);
    }

    room->setPlayerMark(first, "WushuangTarget", 0);
    room->setPlayerMark(second, "WushuangTarget", 0);

    DamageStruct damage;
    damage.card = this;
    if(second->isAlive())
        damage.from = second;
    else
        damage.from = NULL;
    damage.to = first;

    room->damage(damage);
}

Snatch::Snatch(Suit suit, int number):SingleTargetTrick(suit, number, true) {
    setObjectName("snatch");
}

bool Snatch::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;

    if (to_select->isAllNude())
        return false;

    if (to_select == Self)
        return false;

    int distance_limit = 1 + Sanguosha->correctCardTarget(TargetModSkill::DistanceLimit, Self, this);
    int rangefix = 0;
    if (Self->getWeapon() && subcards.contains(Self->getWeapon()->getId())){
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        rangefix += weapon->getRange() - 1;
    }

    if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId()))
        rangefix += 1;

    if (getSkillName() == "jixi")
        rangefix += 1;

    if (Self->distanceTo(to_select, rangefix) > distance_limit)
        return false;

    return true;
}

void Snatch::onEffect(const CardEffectStruct &effect) const{
    if(effect.from->isDead())
        return;
    if(effect.to->isAllNude())
        return;

    Room *room = effect.to->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "hej", objectName());
    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, effect.from->objectName());
    room->obtainCard(effect.from, Sanguosha->getCard(card_id), reason, room->getCardPlace(card_id) != Player::PlaceHand);
}

Dismantlement::Dismantlement(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
        setObjectName("dismantlement");
}

bool Dismantlement::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    if (targets.length() >= total_num)
        return false;

    if (to_select->isAllNude())
        return false;

    if (to_select == Self)
        return false;

    return true;
}

void Dismantlement::onEffect(const CardEffectStruct &effect) const{
    if(effect.from->isDead())
        return;
    if(effect.to->isAllNude())
        return;

    Room *room = effect.to->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "hej", objectName());
    room->throwCard(card_id, room->getCardPlace(card_id) == Player::PlaceDelayedTrick ? NULL : effect.to, effect.from);
}

Indulgence::Indulgence(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("indulgence");

    judge.pattern = QRegExp("(.*):(heart):(.*)");
    judge.good = true;
    judge.reason = objectName();
}

bool Indulgence::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if(!targets.isEmpty())
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void Indulgence::takeEffect(ServerPlayer *target) const{
    target->clearHistory();
    target->skip(Player::Play);
}

Disaster::Disaster(Card::Suit suit, int number)
    :DelayedTrick(suit, number, true)
{
    target_fixed = true;
}

bool Disaster::isAvailable(const Player *player) const{
    if(player->containsTrick(objectName()))
        return false;

    return !player->isProhibited(player, this);
}

Lightning::Lightning(Suit suit, int number):Disaster(suit, number){
    setObjectName("lightning");

    judge.pattern = QRegExp("(.*):(spade):([2-9])");
    judge.good = false;
    judge.reason = objectName();
}

void Lightning::takeEffect(ServerPlayer *target) const{
    DamageStruct damage;
    damage.card = this;
    damage.damage = 3;
    damage.from = NULL;
    damage.to = target;
    damage.nature = DamageStruct::Thunder;

    target->getRoom()->damage(damage);
}


// EX cards

class IceSwordSkill: public WeaponSkill{
public:
    IceSwordSkill():WeaponSkill("IceSword"){
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.card && damage.card->isKindOf("Slash")
           && damage.to->getMark("Equips_of_Others_Nullified_to_You") == 0
           && !damage.to->isNude()
           && !damage.chain && !damage.transfer && player->askForSkillInvoke(objectName(), data)){
                room->setEmotion(player, "weapon/ice_sword");
                int card_id = room->askForCardChosen(player, damage.to, "he", objectName());
                room->throwCard(Sanguosha->getCard(card_id), damage.to, damage.from);

                if(!damage.to->isNude()){
                    card_id = room->askForCardChosen(player, damage.to, "he", objectName());
                    room->throwCard(Sanguosha->getCard(card_id), damage.to, damage.from);
                }

                return true;
        }

        return false;
    }
};

IceSword::IceSword(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("IceSword");
}

class RenwangShieldSkill: public ArmorSkill{
public:
    RenwangShieldSkill():ArmorSkill("RenwangShield"){
        events << SlashEffected;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->isBlack()){
            LogMessage log;
            log.type = "#ArmorNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();
            player->getRoom()->sendLog(log);

            room->setEmotion(player, "armor/renwang_shield");

            return true;
        }else
            return false;
    }
};

RenwangShield::RenwangShield(Suit suit, int number)
    :Armor(suit, number)
{
    setObjectName("RenwangShield");
}

class HorseSkill: public DistanceSkill{
public:
    HorseSkill():DistanceSkill("Horse"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        const Horse* horse = NULL;
        if(from->getOffensiveHorse() && from->getMark("Equips_Nullified_to_Yourself") == 0){
            horse = qobject_cast<const Horse*>(from->getOffensiveHorse()->getRealCard());
            correct += horse->getCorrect();
        }
        if(to->getDefensiveHorse() && to->getMark("Equips_Nullified_to_Yourself") == 0){
            horse = qobject_cast<const Horse*>(to->getDefensiveHorse()->getRealCard());
            correct += horse->getCorrect();
        }

        return correct;
    }
};

StandardCardPackage::StandardCardPackage()
    :Package("standard_cards")
{
    type = Package::CardPack;

    QList<Card*> cards;

    cards << new Slash(Card::Spade, 7)
        << new Slash(Card::Spade, 8)
        << new Slash(Card::Spade, 8)
        << new Slash(Card::Spade, 9)
        << new Slash(Card::Spade, 9)
        << new Slash(Card::Spade, 10)
        << new Slash(Card::Spade, 10)

        << new Slash(Card::Club, 2)
        << new Slash(Card::Club, 3)
        << new Slash(Card::Club, 4)
        << new Slash(Card::Club, 5)
        << new Slash(Card::Club, 6)
        << new Slash(Card::Club, 7)
        << new Slash(Card::Club, 8)
        << new Slash(Card::Club, 8)
        << new Slash(Card::Club, 9)
        << new Slash(Card::Club, 9)
        << new Slash(Card::Club, 10)
        << new Slash(Card::Club, 10)
        << new Slash(Card::Club, 11)
        << new Slash(Card::Club, 11)

        << new Slash(Card::Heart, 10)
        << new Slash(Card::Heart, 10)
        << new Slash(Card::Heart, 11)

        << new Slash(Card::Diamond, 6)
        << new Slash(Card::Diamond, 7)
        << new Slash(Card::Diamond, 8)
        << new Slash(Card::Diamond, 9)
        << new Slash(Card::Diamond, 10)
        << new Slash(Card::Diamond, 13)

        << new Jink(Card::Heart, 2)
        << new Jink(Card::Heart, 2)
        << new Jink(Card::Heart, 13)

        << new Jink(Card::Diamond, 2)
        << new Jink(Card::Diamond, 2)
        << new Jink(Card::Diamond, 3)
        << new Jink(Card::Diamond, 4)
        << new Jink(Card::Diamond, 5)
        << new Jink(Card::Diamond, 6)
        << new Jink(Card::Diamond, 7)
        << new Jink(Card::Diamond, 8)
        << new Jink(Card::Diamond, 9)
        << new Jink(Card::Diamond, 10)
        << new Jink(Card::Diamond, 11)
        << new Jink(Card::Diamond, 11)

        << new Peach(Card::Heart, 3)
        << new Peach(Card::Heart, 4)
        << new Peach(Card::Heart, 6)
        << new Peach(Card::Heart, 7)
        << new Peach(Card::Heart, 8)
        << new Peach(Card::Heart, 9)
        << new Peach(Card::Heart, 12)

        << new Peach(Card::Diamond, 12)

        << new Crossbow(Card::Club)
        << new Crossbow(Card::Diamond)
        << new DoubleSword
        << new QinggangSword
        << new Blade
        << new Spear
        << new Axe
        << new Halberd
        << new KylinBow

        << new EightDiagram(Card::Spade)
        << new EightDiagram(Card::Club);
    
        skills << new DoubleSwordSkill << new QinggangSwordSkill
               << new BladeSkill << new SpearSkill << new AxeSkill << new HalberdSkill
               << new KylinBowSkill << new EightDiagramSkill;

    {
        QList<Card *> horses;
        horses << new DefensiveHorse(Card::Spade, 5)
            << new DefensiveHorse(Card::Club, 5)
            << new DefensiveHorse(Card::Heart, 13)
            << new OffensiveHorse(Card::Heart, 5)
            << new OffensiveHorse(Card::Spade, 13)
            << new OffensiveHorse(Card::Diamond, 13);

        horses.at(0)->setObjectName("JueYing");
        horses.at(1)->setObjectName("DiLu");
        horses.at(2)->setObjectName("ZhuaHuangFeiDian");
        horses.at(3)->setObjectName("ChiTu");
        horses.at(4)->setObjectName("DaYuan");
        horses.at(5)->setObjectName("ZiXing");

        cards << horses;

        skills << new HorseSkill;
    }

    cards << new AmazingGrace(Card::Heart, 3)
        << new AmazingGrace(Card::Heart, 4)
        << new GodSalvation
        << new SavageAssault(Card::Spade, 7)
        << new SavageAssault(Card::Spade, 13)
        << new SavageAssault(Card::Club, 7)
        << new ArcheryAttack
        << new Duel(Card::Spade, 1)
        << new Duel(Card::Club, 1)
        << new Duel(Card::Diamond, 1)
        << new ExNihilo(Card::Heart, 7)
        << new ExNihilo(Card::Heart, 8)
        << new ExNihilo(Card::Heart, 9)
        << new ExNihilo(Card::Heart, 11)
        << new Snatch(Card::Spade, 3)
        << new Snatch(Card::Spade, 4)
        << new Snatch(Card::Spade, 11)
        << new Snatch(Card::Diamond, 3)
        << new Snatch(Card::Diamond, 4)
        << new Dismantlement(Card::Spade, 3)
        << new Dismantlement(Card::Spade, 4)
        << new Dismantlement(Card::Spade, 12)
        << new Dismantlement(Card::Club, 3)
        << new Dismantlement(Card::Club, 4)
        << new Dismantlement(Card::Heart, 12)
        << new Collateral(Card::Club, 12)
        << new Collateral(Card::Club, 13)
        << new Nullification(Card::Spade, 11)
        << new Nullification(Card::Club, 12)
        << new Nullification(Card::Club, 13)
        << new Indulgence(Card::Spade, 6)
        << new Indulgence(Card::Club, 6)
        << new Indulgence(Card::Heart, 6)
        << new Lightning(Card::Spade, 1);

    foreach(Card *card, cards)
        card->setParent(this);
}

StandardExCardPackage::StandardExCardPackage()
    :Package("standard_ex_cards")
{
    QList<Card *> cards;
    cards << new IceSword(Card::Spade, 2)
        << new RenwangShield(Card::Club, 2)
        << new Lightning(Card::Heart, 12)
        << new Nullification(Card::Diamond, 12);

    skills << new RenwangShieldSkill << new IceSwordSkill;

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(StandardCard)
ADD_PACKAGE(StandardExCard)
