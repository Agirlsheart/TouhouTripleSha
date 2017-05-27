#include "player.h"
#include "engine.h"
#include "room.h"
#include "client.h"
#include "standard.h"
#include "settings.h"

Player::Player(QObject *parent)
    : QObject(parent), owner(false), general(NULL), general2(NULL),
      m_gender(General::Sexless), hp(-1), max_hp(-1),
      state("online"), seat(0), alive(true),
      phase(NotActive),
      weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL), treasure(NULL),
      face_up(true), chained(false), removed(false),
      role_shown(false), pile_open(QMap<QString, QStringList>())
{
}

void Player::setScreenName(const QString &screen_name) {
    this->screen_name = screen_name;
}

QString Player::screenName() const{
    return screen_name;
}

bool Player::isOwner() const{
    return owner;
}

void Player::setOwner(bool owner) {
    if (this->owner != owner) {
        this->owner = owner;
        emit owner_changed(owner);
    }
}

bool Player::hasShownRole() const {
    return role_shown;
}

void Player::setShownRole(bool shown) {
    this->role_shown = shown;
}

void Player::setHp(int hp) {
    if (this->hp != hp) {
        this->hp = hp;
        emit hp_changed();
    }
}

int Player::getHp() const{
    return hp;
}

int Player::getMaxHp() const{
    return max_hp;
}

void Player::setMaxHp(int max_hp) {
    if (this->max_hp == max_hp)
        return;
    this->max_hp = max_hp;
    if (hp > max_hp)
        hp = max_hp;
    emit hp_changed();
}

int Player::getLostHp() const{
    return max_hp - qMax(hp, 0);
}

bool Player::isWounded() const
{
    foreach (const Player *p, getAliveSiblings()) {
        if (p->getPhase() != NotActive && p->hasSkill("ikguiming") && p->getHp() <= 2 && getKingdom() == "kaze")
            return true;
    }

    if (hp < 0)
        return true;
    else
        return hp < max_hp;
}

int Player::getGender() const{
    return (int)m_gender;
}

void Player::setGender(int gender) {
    m_gender = (General::Gender)gender;
}

bool Player::isMale() const{
    return m_gender == General::Male;
}

bool Player::isFemale() const{
    return m_gender == General::Female;
}

bool Player::isNeuter() const{
    return m_gender == General::Neuter;
}

int Player::getSeat() const{
    return seat;
}

void Player::setSeat(int seat) {
    this->seat = seat;
}

bool Player::isAdjacentTo(const Player *another) const{
    return getNextAlive() == another
        || another->getNextAlive() == this;
}

bool Player::isAlive() const{
    return alive;
}

bool Player::isDead() const{
    return !alive;
}

void Player::setAlive(bool alive) {
    this->alive = alive;
}

QString Player::getFlags() const{
    return QStringList(flags.toList()).join("|");
}

QStringList Player::getFlagList() const{
    return QStringList(flags.toList());
}

void Player::setFlags(const QString &flag) {
    if (flag == ".") {
        clearFlags();
        return;
    }
    static QChar unset_symbol('-');
    if (flag.startsWith(unset_symbol)) {
        QString copy = flag;
        copy.remove(unset_symbol);
        flags.remove(copy);
    } else {
        flags.insert(flag);
    }
}

bool Player::hasFlag(const QString &flag) const{
    return flags.contains(flag);
}

void Player::clearFlags() {
    flags.clear();
}

int Player::getAttackRange(bool include_weapon) const{
    int original_range = 1;
    if (hasFlag("InfinityAttackRange") || getMark("InfinityAttackRange") > 0) original_range = 10000; // Actually infinity
    int weapon_range = 0;
    if (include_weapon) {
        if (!weapon && hasSkill("thsilian"))
            weapon_range = 3;
        else {
            WrappedCard *wp = weapon;
            if (!wp && !hasOwnerOnlySkill(true) && getMark("@technology") > 0) {
                foreach (const Player *p, getAliveSiblings()) {
                    if (p->getMark("thmicaisource") > 0) {
                        wp = p->weapon;
                        break;
                    }
                }
            }

            if (wp) {
                const Weapon *card = qobject_cast<const Weapon *>(wp->getRealCard());
                Q_ASSERT(card);
                weapon_range = card->getRange();
            }
        }
    }
    int extra_range = 0;
    if (hasFlag("thhuanlong1"))
        extra_range += 1;
    if (hasLordSkill("thfeizhan")) {
        foreach (const Player *p, getAliveSiblings()) {
            if (p->getKingdom() == "hana")
                ++extra_range;
        }
    }
    return qMax(original_range, weapon_range) + extra_range;
}

bool Player::inMyAttackRange(const Player *other, int distance_fix) const
{
    if (this == other)
        return false;
    if (distanceTo(other, distance_fix) == -1)
        return false;
    if (attack_range_pair.contains(other))
        return true;
    if (hasSkill("rhmangti") && other->getHandcardNum() < 2)
        return true;
    return distanceTo(other, distance_fix) <= getAttackRange();
}

void Player::setFixedDistance(const Player *player, int distance) {
    if (distance < 0)
        fixed_distance.remove(player, -distance);
    else
        fixed_distance.insert(player, distance);
}

void Player::removeFixedDistance(const Player *player, int distance) {
    fixed_distance.remove(player, distance);
}

void Player::insertAttackRangePair(const Player *player) {
    attack_range_pair.append(player);
}

void Player::removeAttackRangePair(const Player *player) {
    attack_range_pair.removeOne(player);
}

int Player::originalRightDistanceTo(const Player *other) const{
    int right = 0;
    Player *next_p = parent()->findChild<Player *>(objectName());
    while (next_p != other) {
        next_p = next_p->getNextAlive();
        ++right;
    }
    return right;
}

int Player::distanceTo(const Player *other, int distance_fix) const{
    if (this == other || isDead() || other->isDead())
        return 0;

    if (isRemoved() || other->isRemoved())
        return -1;

    if ((hasSkill("thguimen") && other->isChained())
            || (other->hasSkill("thguimen") && isChained()))
        return 1;

    if (hasSkill("ikhuisuo") && other->getHp() < getHp())
        return 1;

    if (fixed_distance.contains(other)) {
        QList<int> distance_list = fixed_distance.values(other);
        int min = 10000;
        foreach (int d, distance_list) {
            if (min > d)
                min = d;
        }

        return min;
    }

    int right = originalRightDistanceTo(other);
    int left = aliveCount(false) - right;
    int distance = qMin(left, right);

    distance += Sanguosha->correctDistance(this, other);
    distance += distance_fix;

    // keep the distance >=1
    if (distance < 1)
        distance = 1;

    if (distance == 1 && other->hasSkill("jnjicha") && other->getHp() == 1 && getHp() != 1)
        distance = 2;

    return distance;
}

void Player::setGeneral(const General *new_general) {
    if (this->general != new_general) {
        this->general = new_general;

        if (new_general && kingdom.isEmpty())
            setKingdom(new_general->getKingdom());

        emit general_changed();
    }
}

void Player::setGeneralName(const QString &general_name) {
    const General *new_general = Sanguosha->getGeneral(general_name);
    Q_ASSERT(general_name.isNull() || general_name.isEmpty() || new_general != NULL);
    setGeneral(new_general);
}

QString Player::getGeneralName() const{
    if (general)
        return general->objectName();
    else
        return QString();
}

void Player::setGeneral2Name(const QString &general_name) {
    const General *new_general = Sanguosha->getGeneral(general_name);
    if (general2 != new_general) {
        general2 = new_general;

        emit general2_changed();
    }
}

QString Player::getGeneral2Name() const{
    if (general2)
        return general2->objectName();
    else
        return QString();
}

const General *Player::getGeneral2() const{
    return general2;
}

QString Player::getState() const{
    return state;
}

void Player::setState(const QString &state) {
    if (this->state != state) {
        this->state = state;
        emit state_changed();
    }
}

void Player::setRole(const QString &role) {
    if (this->role != role) {
        this->role = role;
        emit role_changed(role);
    }
}

QString Player::getRole() const{
    return role;
}

Player::Role Player::getRoleEnum() const{
    static QMap<QString, Role> role_map;
    if (role_map.isEmpty()) {
        role_map.insert("lord", Lord);
        role_map.insert("loyalist", Loyalist);
        role_map.insert("rebel", Rebel);
        role_map.insert("renegade", Renegade);
    }

    return role_map.value(role);
}

const General *Player::getAvatarGeneral() const{
    if (general)
        return general;

    QString general_name = property("avatar").toString();
    if (general_name.isEmpty())
        return NULL;
    return Sanguosha->getGeneral(general_name);
}

const General *Player::getGeneral() const{
    return general;
}

bool Player::isLord() const{
    return getRole() == "lord";
}

bool Player::hasSkill(const QString &skill_name, bool include_lose) const{
    if (!include_lose) {
        if (!hasEquipSkill(skill_name)) {
            const Skill *skill = Sanguosha->getSkill(skill_name);
            if (skill && skill_name != "thyanmeng" && skill_name != "thxuanyan" && !Sanguosha->correctSkillValidity(this, skill))
                return false;
        }
    }
    return skills.contains(skill_name)
            || acquired_skills.contains(skill_name)
            || extra_skills.contains(skill_name);
}

bool Player::hasSkills(const QString &skill_name, bool include_lose) const{
    foreach (QString skill, skill_name.split("|")) {
        bool checkpoint = true;
        foreach (QString sk, skill.split("+")) {
            if (!hasSkill(sk, include_lose)) {
                checkpoint = false;
                break;
            }
        }
        if (checkpoint) return true;
    }
    return false;
}

bool Player::hasInnateSkill(const QString &skill_name) const{
    if (general && general->hasSkill(skill_name))
        return true;

    if (general2 && general2->hasSkill(skill_name))
        return true;

    return false;
}

bool Player::hasLordSkill(const QString &skill_name, bool include_lose) const{
    if (!isLord() && hasSkill("ikshengzun")) {
        foreach (const Player *player, getAliveSiblings()) {
            if (player->isLord()) {
                if (player->hasLordSkill(skill_name, true))
                    return true;
                break;
            }
        }
    }

    if (!hasSkill(skill_name, include_lose))
        return false;

    if (acquired_skills.contains(skill_name) || extra_skills.contains(skill_name))
        return true;

    QString mode = getGameMode();
    if (mode == "06_3v3" || mode == "06_XMode" || mode == "02_1v1" || Config.value("WithoutLordskill", false).toBool())
        return false;

    if (ServerInfo.EnableHegemony)
        return false;

    if (isLord())
        return skills.contains(skill_name);

    return false;
}

bool Player::hasOwnerOnlySkill(bool include_lose) const{
    foreach (const Skill *skill, getVisibleSkills(include_lose)) {
        if (skill->isOwnerOnlySkill())
            return true;
    }
    return false;
}

void Player::acquireSkill(const QString &skill_name, const bool &is_extra)
{
    if (!is_extra)
        acquired_skills.append(skill_name);
    else
        extra_skills.append(skill_name);
}

void Player::detachSkill(const QString &skill_name, const bool &is_extra)
{
    if (!is_extra)
        acquired_skills.removeOne(skill_name);
    else
        extra_skills.removeOne(skill_name);
}

void Player::detachAllSkills() {
    acquired_skills.clear();
    extra_skills.clear();
}

void Player::addSkill(const QString &skill_name) {
    skills << skill_name;
}

void Player::loseSkill(const QString &skill_name) {
    skills.removeOne(skill_name);
}

QString Player::getPhaseString(Phase p) const{
    if (p == PhaseNone) p = phase;
    switch (p) {
    case RoundStart: return "round_start";
    case Start: return "start";
    case Judge: return "judge";
    case Draw: return "draw";
    case Play: return "play";
    case Discard: return "discard";
    case Finish: return "finish";
    case NotActive:
    default:
            return "not_active";
    }
}

void Player::setPhaseString(const QString &phase_str) {
    static QMap<QString, Phase> phase_map;
    if (phase_map.isEmpty()) {
        phase_map.insert("round_start", RoundStart);
        phase_map.insert("start", Start);
        phase_map.insert("judge", Judge);
        phase_map.insert("draw", Draw);
        phase_map.insert("play", Play);
        phase_map.insert("discard", Discard);
        phase_map.insert("finish", Finish);
        phase_map.insert("not_active", NotActive);
    }

    setPhase(phase_map.value(phase_str, NotActive));
}

void Player::setEquip(WrappedCard *equip) {
    const EquipCard *card = qobject_cast<const EquipCard *>(equip->getRealCard());
    Q_ASSERT(card != NULL);
    switch(card->location()) {
    case EquipCard::WeaponLocation: weapon = equip; break;
    case EquipCard::ArmorLocation: armor = equip; break;
    case EquipCard::DefensiveHorseLocation: defensive_horse = equip; break;
    case EquipCard::OffensiveHorseLocation: offensive_horse = equip; break;
    case EquipCard::TreasureLocation: treasure = equip; break;
    }
}

void Player::removeEquip(WrappedCard *equip) {
    const EquipCard *card = qobject_cast<const EquipCard *>(equip->getRealCard());
    Q_ASSERT(card != NULL);
    if (!card)
        card = qobject_cast<const EquipCard *>(Sanguosha->getEngineCard(equip->getEffectiveId()));
    switch(card->location()) {
    case EquipCard::WeaponLocation: weapon = NULL; break;
    case EquipCard::ArmorLocation: armor = NULL; break;
    case EquipCard::DefensiveHorseLocation: defensive_horse = NULL; break;
    case EquipCard::OffensiveHorseLocation: offensive_horse = NULL; break;
    case EquipCard::TreasureLocation: treasure = NULL; break;
    }
}

bool Player::hasEquip(const Card *card) const{
    Q_ASSERT(card != NULL);
    int weapon_id = -1, armor_id = -1, def_id = -1, off_id = -1, tr_id = -1;
    if (weapon) weapon_id = weapon->getEffectiveId();
    if (armor) armor_id = armor->getEffectiveId();
    if (defensive_horse) def_id = defensive_horse->getEffectiveId();
    if (offensive_horse) off_id = offensive_horse->getEffectiveId();
    if (treasure) tr_id = treasure->getEffectiveId();
    QList<int> ids;
    if (card->isVirtualCard())
        ids << card->getSubcards();
    else
        ids << card->getId();
    if (ids.isEmpty()) return false;
    foreach (int id, ids) {
        if (id != weapon_id && id != armor_id && id != def_id && id != off_id && id != tr_id)
            return false;
    }
    return true;
}

bool Player::hasEquip() const{
    return weapon != NULL || armor != NULL || defensive_horse != NULL || offensive_horse != NULL || treasure != NULL;
}

WrappedCard *Player::getWeapon() const{
    return weapon;
}

WrappedCard *Player::getArmor() const{
    return armor;
}

WrappedCard *Player::getDefensiveHorse() const{
    return defensive_horse;
}

WrappedCard *Player::getOffensiveHorse() const{
    return offensive_horse;
}

WrappedCard *Player::getTreasure() const{
    return treasure;
}

QList<const Card *> Player::getEquips() const{
    QList<const Card *> equips;
    if (weapon)
        equips << weapon;
    if (armor)
        equips << armor;
    if (defensive_horse)
        equips << defensive_horse;
    if (offensive_horse)
        equips << offensive_horse;
    if (treasure)
        equips << treasure;

    return equips;
}

const EquipCard *Player::getEquip(int index) const{
    WrappedCard *equip;
    switch (index) {
    case 0: equip = weapon; break;
    case 1: equip = armor; break;
    case 2: equip = defensive_horse; break;
    case 3: equip = offensive_horse; break;
    case 4: equip = treasure; break;
    default:
            return NULL;
    }
    if (equip != NULL)
        return qobject_cast<const EquipCard *>(equip->getRealCard());

    return NULL;
}

bool Player::hasWeapon(const QString &weapon_name) const {
    if (getMark("Equips_Nullified_to_Yourself") > 0) return false;

    if (weapon == NULL && alive) {
        if (weapon_name == "blade" && hasSkill("thsilian"))
            return true;
    }

    WrappedCard *wp = weapon;
    if (!wp && !hasOwnerOnlySkill(true) && getMark("@technology") > 0) {
        foreach (const Player *p, getAliveSiblings()) {
            if (p->getMark("thmicaisource") > 0) {
                wp = p->weapon;
                break;
            }
        }
    }
    if (!wp)
        return false;
    if (wp->objectName() == weapon_name || wp->isKindOf(weapon_name.toStdString().c_str()))
        return true;
    const Card *real_weapon = Sanguosha->getEngineCard(wp->getEffectiveId());
    return real_weapon->objectName() == weapon_name || real_weapon->isKindOf(weapon_name.toStdString().c_str());
}

bool Player::hasArmorEffect(const QString &armor_name) const{
    if (!tag["Qinggang"].toStringList().isEmpty() || getMark("Armor_Nullified") > 0
        || getMark("Equips_Nullified_to_Yourself") > 0)
        return false;

    const Player *current = NULL;
    foreach (const Player *p, getAliveSiblings()) {
        if (p->getPhase() != Player::NotActive) {
            current = p;
            break;
        }
    }
    if (current && current->hasSkill("ikshensha")) {
        bool alladj = true;
        foreach (const Player *p, current->getAliveSiblings()) {
            if (current->distanceTo(p) != 1) {
                alladj = false;
                break;
            }
        }
        if (alladj) return false;
    }

    if (armor == NULL && alive) {
        if (armor_name == "eight_diagram" && hasSkill("ikjingnie"))
            return true;
        if (armor_name == "eight_diagram" && hasSkill("iklinglong"))
            return true;
        if (armor_name == "vine" && hasSkill("bossmanjia"))
            return true;
    }

    WrappedCard *am = armor;
    if (!am && !hasOwnerOnlySkill(true) && getMark("@technology") > 0) {
        foreach (const Player *p, getAliveSiblings()) {
            if (p->getMark("thmicaisource") > 0) {
                am = p->armor;
                break;
            }
        }
    }
    if (!am)
        return false;
    if (am->objectName() == armor_name || am->isKindOf(armor_name.toStdString().c_str()))
        return true;
    const Card *real_armor = Sanguosha->getEngineCard(am->getEffectiveId());
    return real_armor->objectName() == armor_name || real_armor->isKindOf(armor_name.toStdString().c_str());
}

bool Player::hasTreasure(const QString &treasure_name) const{
    if (!treasure || getMark("Equips_Nullified_to_Yourself") > 0) return false;
    if (treasure->objectName() == treasure_name || treasure->isKindOf(treasure_name.toStdString().c_str())) return true;
    const Card *real_treasure = Sanguosha->getEngineCard(treasure->getEffectiveId());
    return real_treasure->objectName() == treasure_name || real_treasure->isKindOf(treasure_name.toStdString().c_str());
}

QList<const Card *> Player::getJudgingArea() const{
    QList<const Card *>cards;
    foreach (int card_id, judging_area)
        cards.append(Sanguosha->getCard(card_id));
    return cards;
}

QList<int> Player::getJudgingAreaID() const{
    return judging_area;
}

Player::Phase Player::getPhase() const{
    return phase;
}

void Player::setPhase(Phase phase) {
    this->phase = phase;
    emit phase_changed();
}

bool Player::faceUp() const{
    return face_up;
}

void Player::setFaceUp(bool face_up) {
    if (this->face_up != face_up) {
        this->face_up = face_up;
        emit state_changed();
    }
}

int Player::getMaxCards() const{
    int origin = Sanguosha->correctMaxCards(this, true);
    if (origin < 0)
        origin = qMax(hp, 0);
    int rule = 0, total = 0, extra = 0;
    if (Config.MaxHpScheme == 3 && general2) {
        total = general->getMaxHp() + general2->getMaxHp();
        if (total % 2 != 0 && getMark("AwakenLostMaxHp") == 0)
            rule = 1;
    }
    extra += Sanguosha->correctMaxCards(this);

    return qMax(origin + rule + extra, 0);
}

QString Player::getKingdom() const{
    if (kingdom.isEmpty() && general)
        return general->getKingdom();
    else
        return kingdom;
}

void Player::setKingdom(const QString &kingdom) {
    if (this->kingdom != kingdom) {
        this->kingdom = kingdom;
        emit kingdom_changed();
    }
}

bool Player::isKongcheng() const{
    return getHandcardNum() == 0;
}

bool Player::isNude() const{
    return isKongcheng() && !hasEquip();
}

bool Player::isAllNude() const{
    return isNude() && judging_area.isEmpty();
}

bool Player::canDiscard(const Player *to, const QString &flags) const{
    static QChar handcard_flag('h');
    static QChar equip_flag('e');
    static QChar judging_flag('j');

    if (this->isDead() || to->isDead()) return false;
    if (flags.contains(handcard_flag) && !to->isKongcheng()) return true;
    if (flags.contains(judging_flag) && !to->getJudgingArea().isEmpty()) return true;
    if (flags.contains(equip_flag)) {
        if (to->getDefensiveHorse() || to->getOffensiveHorse() || to->getTreasure()) return true;
        if ((to->getWeapon() || to->getArmor()) && (!to->hasSkill("ikhugu") || this == to)) return true;
    }
    return false;
}

bool Player::canDiscard(const Player *to, int card_id) const{
    if (this->isDead() || to->isDead()) return false;
    if (to->hasSkill("ikhugu") && this != to) {
        if ((to->getWeapon() && card_id == to->getWeapon()->getEffectiveId())
            || (to->getArmor() && card_id == to->getArmor()->getEffectiveId()))
            return false;
    } else if (this == to) {
        if (!getJudgingAreaID().contains(card_id) && isJilei(Sanguosha->getCard(card_id)))
            return false;
    }
    return true;
}

void Player::addDelayedTrick(const Card *trick) {
    judging_area << trick->getId();
}

void Player::removeDelayedTrick(const Card *trick) {
    int index = judging_area.indexOf(trick->getId());
    if (index >= 0)
        judging_area.removeAt(index);
}

bool Player::containsTrick(const QString &trick_name) const{
    foreach (int trick_id, judging_area) {
        WrappedCard *trick = Sanguosha->getWrappedCard(trick_id);
        if (trick->objectName() == trick_name)
            return true;
    }
    return false;
}

bool Player::isChained() const{
    return chained;
}

void Player::setChained(bool chained) {
    if (this->chained != chained) {
        this->chained = chained;
        emit state_changed();
    }
}

bool Player::isRemoved() const{
    return removed;
}

void Player::setRemoved(bool removed) {
    if (this->removed != removed) {
        this->removed = removed;
        emit removedChanged();
    }
}

void Player::addMark(const QString &mark, int add_num) {
    int value = marks.value(mark, 0);
    value += add_num;
    setMark(mark, value);
}

void Player::removeMark(const QString &mark, int remove_num) {
    int value = marks.value(mark, 0);
    value -= remove_num;
    value = qMax(0, value);
    setMark(mark, value);
}

void Player::setMark(const QString &mark, int value) {
    if (marks[mark] != value)
        marks[mark] = value;
}

int Player::getMark(const QString &mark) const{
    return marks.value(mark, 0);
}

QStringList Player::getMarkNames() const{
    return marks.keys();
}

bool Player::canSlash(const Player *other, const Card *slash, bool distance_limit,
                      int rangefix, const QList<const Player *> &others) const{
    if (!other->isAlive())
        return false;

    if (!hasFlag("IkXiashanUse") && !hasFlag("ThXuyouUse") && other == this)
        return false;

    Slash *newslash = new Slash(Card::NoSuit, 0);
    newslash->deleteLater();
#define THIS_SLASH (slash == NULL ? newslash : slash)
    if (isProhibited(other, THIS_SLASH, others))
        return false;

    if (distance_limit)
        return inMyAttackRange(other, rangefix - Sanguosha->correctCardTarget(TargetModSkill::DistanceLimit, this, THIS_SLASH));
    else
        return true;
#undef THIS_SLASH
}

bool Player::canSlash(const Player *other, bool distance_limit, int rangefix, const QList<const Player *> &others) const{
    return canSlash(other, NULL, distance_limit, rangefix, others);
}

int Player::getCardCount(bool include_equip, bool include_judging) const{
    int count = getHandcardNum();
    if (include_equip) {
        if (weapon) count++;
        if (armor) count++;
        if (defensive_horse) count++;
        if (offensive_horse) count++;
        if (treasure) count++;
    }
    if (include_judging)
        count += judging_area.length();
    return count;
}

QList<int> Player::getPile(const QString &pile_name) const{
    return piles[pile_name];
}

QStringList Player::getPileNames() const{
    QStringList names;
    foreach (QString pile_name, piles.keys())
        names.append(pile_name);
    return names;
}

QString Player::getPileName(int card_id) const{
    foreach (QString pile_name, piles.keys()) {
        QList<int> pile = piles[pile_name];
        if (pile.contains(card_id))
            return pile_name;
    }

    return QString();
}

bool Player::pileOpen(const QString &pile_name, const QString &player) const {
    return pile_open[pile_name].contains(player);
}

void Player::setPileOpen(const QString &pile_name, const QString &player) {
    if (pile_open[pile_name].contains(player)) return;
    pile_open[pile_name].append(player);
}

void Player::addHistory(const QString &name, int times) {
    history[name] += times;
}

int Player::getSlashCount() const{
    return history.value("Slash", 0)
           + history.value("ThunderSlash", 0)
           + history.value("FireSlash", 0);
}

void Player::clearHistory(const QString &name) {
    if (name.isEmpty())
        history.clear();
    else
        history.remove(name);
}

bool Player::hasUsed(const QString &card_class) const{
    return history.value(card_class, 0) > 0;
}

int Player::usedTimes(const QString &card_class) const{
    return history.value(card_class, 0);
}

bool Player::hasEquipSkill(const QString &skill_name) const{
    if (weapon) {
        const Weapon *weaponc = qobject_cast<const Weapon *>(weapon->getRealCard());
        if (Sanguosha->getSkill(weaponc) && Sanguosha->getSkill(weaponc)->objectName() == skill_name)
            return true;
    }
    if (armor) {
        const Armor *armorc = qobject_cast<const Armor *>(armor->getRealCard());
        if (Sanguosha->getSkill(armorc) && Sanguosha->getSkill(armorc)->objectName() == skill_name)
            return true;
    }
    if (treasure) {
        const Treasure *treasurec = qobject_cast<const Treasure *>(treasure->getRealCard());
        if (Sanguosha->getSkill(treasurec) && Sanguosha->getSkill(treasurec)->objectName() == skill_name)
            return true;
    }
    return false;
}

QSet<const TriggerSkill *> Player::getTriggerSkills(bool include_extra) const{
    QSet<const TriggerSkill *> skillList;
    QStringList skill_list = skills + acquired_skills;
    if (include_extra)
        skill_list += extra_skills;
    foreach (QString skill_name, skill_list.toSet()) {
        const TriggerSkill *skill = Sanguosha->getTriggerSkill(skill_name);
        if (skill && !hasEquipSkill(skill->objectName()))
            skillList << skill;
    }

    return skillList;
}

QSet<const Skill *> Player::getSkills(bool include_equip, bool visible_only, bool include_extra) const
{
    return getSkillList(include_equip, visible_only, include_extra).toSet();
}

QList<const Skill *> Player::getSkillList(bool include_equip, bool visible_only, bool include_extra) const
{
    QList<const Skill *> skillList;
    QStringList skill_list = skills + acquired_skills;
    if (include_extra)
        skill_list += extra_skills;
    foreach (QString skill_name, skill_list) {
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if (skill && !skillList.contains(skill)
            && (include_equip || !hasEquipSkill(skill->objectName()))
            && (!visible_only || skill->isVisible()))
            skillList << skill;
    }

    return skillList;
}

QSet<const Skill *> Player::getVisibleSkills(bool include_equip, bool include_extra) const{
    return getVisibleSkillList(include_equip, include_extra).toSet();
}

QList<const Skill *> Player::getVisibleSkillList(bool include_equip, bool include_extra) const{
    return getSkillList(include_equip, true, include_extra);
}

QStringList Player::getAcquiredSkills() const
{
    return acquired_skills;
}

QStringList Player::getExtraSkills() const
{
    return extra_skills;
}

QString Player::getSkillDescription() const{
    QString description = QString();
    QList<const Skill *> skill_list = getVisibleSkillList(false, true);
    QList<const Skill *> basara_list;
    if (getGeneralName() == "anjiang" || getGeneral2Name() == "anjiang") {
        QString basara = property("basara_generals").toString();
        if (!basara.isEmpty()) {
            QStringList basaras = basara.split("+");
            foreach (QString basara_gen, basaras) {
                const General *general = Sanguosha->getGeneral(basara_gen);
                if (general) basara_list.append(general->getVisibleSkillList());
            }
        }
    }

    foreach (const Skill *skill, skill_list + basara_list) {
        if (skill->isAttachedLordSkill() || (!hasSkill(skill->objectName()) && !basara_list.contains(skill)))
            continue;
        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription(getSkillStep(skill->objectName()));
        desc.replace("\n", "<br/>");
        description.append(QString("<b>%1</b>: %2 <br/> <br/>").arg(skill_name).arg(desc));
    }

    if (description.isEmpty()) description = tr("No skills");

    if (general) {
        QString name_origin = Sanguosha->translate("#" + general->objectName());
        name_origin.append(QString(" - %1").arg(Sanguosha->translate(general->objectName())));
        QString color_str = "#000000";
        if (isMale()) {
            color_str = "#F08947";
            name_origin.append(QString(" - %1").arg(Sanguosha->translate("male")));
        } else if (isFemale()) {
            color_str = "#87519C";
            name_origin.append(QString(" - %1").arg(Sanguosha->translate("female")));
        }
        QString name = QString("<font color=%1><b>%2</b></font>").arg(color_str).arg(name_origin);
        name.append("<br/> <br/>");
        description.prepend(name);
    }

    return description;
}

bool Player::isProhibited(const Player *to, const Card *card, const QList<const Player *> &others) const{
    return Sanguosha->isProhibited(this, to, card, others);
}

bool Player::canSlashWithoutCrossbow(const Card *slash) const{
    Slash *newslash = new Slash(Card::NoSuit, 0);
    newslash->deleteLater();
#define THIS_SLASH (slash == NULL ? newslash : slash)
    int slash_count = getSlashCount();
    int valid_slash_count = 1;
    valid_slash_count += Sanguosha->correctCardTarget(TargetModSkill::Residue, this, THIS_SLASH);
    return slash_count < valid_slash_count;
#undef THIS_SLASH
}

void Player::setCardLimitation(const QString &limit_list, const QString &pattern, bool single_turn) {
    QStringList limit_type = limit_list.split(",");
    QString _pattern = pattern;
    if (!pattern.endsWith("$1") && !pattern.endsWith("$0")) {
        QString symb = single_turn ? "$1" : "$0";
        _pattern = _pattern + symb;
    }
    foreach (QString limit, limit_type) {
        Card::HandlingMethod method = Sanguosha->getCardHandlingMethod(limit);
        card_limitation[method] << _pattern;
    }
}

void Player::removeCardLimitation(const QString &limit_list, const QString &pattern) {
    QStringList limit_type = limit_list.split(",");
    QString _pattern = pattern;
    if (!_pattern.endsWith("$1") && !_pattern.endsWith("$0"))
        _pattern = _pattern + "$0";
    foreach (QString limit, limit_type) {
        Card::HandlingMethod method = Sanguosha->getCardHandlingMethod(limit);
        card_limitation[method].removeOne(_pattern);
    }
}

void Player::clearCardLimitation(bool single_turn) {
    QList<Card::HandlingMethod> limit_type;
    limit_type << Card::MethodUse << Card::MethodResponse << Card::MethodDiscard
               << Card::MethodRecast << Card::MethodPindian;
    foreach (Card::HandlingMethod method, limit_type) {
        QStringList limit_patterns = card_limitation[method];
        foreach (QString pattern, limit_patterns) {
            if (!single_turn || pattern.endsWith("$1"))
                card_limitation[method].removeAll(pattern);
        }
    }
}

bool Player::isCardLimited(const Card *card, Card::HandlingMethod method, bool isHandcard) const{
    if (method == Card::MethodNone)
        return false;
    if (card->getTypeId() == Card::TypeSkill && method == card->getHandlingMethod()) {
        foreach (int card_id, card->getSubcards()) {
            const Card *c = Sanguosha->getCard(card_id);
            foreach (QString pattern, card_limitation[method]) {
                QString _pattern = pattern.split("$").first();
                if (isHandcard)
                    _pattern.replace("hand", ".");
                ExpPattern p(_pattern);
                if (p.match(this, c)) return true;
            }
        }
    } else {
        foreach (QString pattern, card_limitation[method]) {
            QString _pattern = pattern.split("$").first();
            if (isHandcard)
                _pattern.replace("hand", ".");
            ExpPattern p(_pattern);
            if (p.match(this, card)) return true;
        }
    }

    return false;
}

void Player::addQinggangTag(const Card *card) {
    QStringList qinggang = this->tag["Qinggang"].toStringList();
    qinggang.append(card->toString());
    this->tag["Qinggang"] = QVariant::fromValue(qinggang);
}

void Player::removeQinggangTag(const Card *card) {
    QStringList qinggang = this->tag["Qinggang"].toStringList();
    if (!qinggang.isEmpty()) {
        qinggang.removeAll(card->toString());
        this->tag["Qinggang"] = qinggang;
    }
    QStringList zhenyao = this->tag["RhZhenyao"].toStringList();
    if (!zhenyao.isEmpty()) {
        zhenyao.removeAll(card->toString());
        this->tag["RhZhenyao"] = zhenyao;
    }
}

void Player::addZhenyaoTag(const Card *card) {
    QStringList zhenyao = this->tag["RhZhenyao"].toStringList();
    zhenyao.append(card->toString());
    this->tag["RhZhenyao"] = QVariant::fromValue(zhenyao);
}

void Player::copyFrom(Player *p)
{
    Player *b = this;
    Player *a = p;

    b->marks            = QMap<QString, int>(a->marks);
    b->piles            = QMap<QString, QList<int> >(a->piles);
    b->acquired_skills  = QStringList(a->acquired_skills);
    b->extra_skills     = QStringList(a->extra_skills);
    b->flags            = QSet<QString>(a->flags);
    b->history          = QHash<QString, int>(a->history);
    b->m_gender         = a->m_gender;

    b->hp               = a->hp;
    b->max_hp           = a->max_hp;
    b->kingdom          = a->kingdom;
    b->role             = a->role;
    b->role_shown       = a->role_shown;
    b->seat             = a->seat;
    b->alive            = a->alive;

    b->phase            = a->phase;
    b->weapon           = a->weapon;
    b->armor            = a->armor;
    b->defensive_horse  = a->defensive_horse;
    b->offensive_horse  = a->offensive_horse;
    b->treasure         = a->treasure;
    b->face_up          = a->face_up;
    b->chained          = a->chained;
    b->removed          = a->removed;
    b->judging_area     = QList<int>(a->judging_area);
    b->fixed_distance   = QMultiHash<const Player *, int>(a->fixed_distance);
    b->card_limitation  = QMap<Card::HandlingMethod, QStringList>(a->card_limitation);

    b->tag              = QVariantMap(a->tag);
}

QList<const Player *> Player::getSiblings() const{
    QList<const Player *> siblings;
    if (parent()) {
        siblings = parent()->findChildren<const Player *>();
        siblings.removeOne(this);
    }
    return siblings;
}

QList<const Player *> Player::getAliveSiblings() const{
    QList<const Player *> siblings = getSiblings();
    foreach (const Player *p, siblings) {
        if (!p->isAlive())
            siblings.removeOne(p);
    }
    return siblings;
}

bool Player::isNostalGeneral(const Player *p, const QString &general_name) {
    return p->getGeneralName() == "nos_" + general_name
           || (p->getGeneralName() != general_name && p->getGeneral2Name() == "nos_" + general_name);
}

void Player::setNext(Player *next)
{
    this->next = next->objectName();
}

void Player::setNext(const QString &next)
{
    this->next = next;
}

Player *Player::getNext(bool ignoreRemoved) const
{
    Player *next_p = parent()->findChild<Player *>(next);
    if (ignoreRemoved && next_p->isRemoved())
        return next_p->getNext(ignoreRemoved);
    return next_p;
}

QString Player::getNextName() const
{
    return next;
}

Player *Player::getLast(bool ignoreRemoved) const
{
    foreach(Player *p, parent()->findChildren<Player *>())
        if (p->getNext(ignoreRemoved) == this)
            return p;
    return NULL;
}

Player *Player::getNextAlive(int n, bool ignoreRemoved) const
{
    bool hasAlive = (aliveCount(!ignoreRemoved) > 0);
    Player *next = parent()->findChild<Player *>(objectName());
    if (!hasAlive) return next;
    for (int i = 0; i < n; ++i) {
        do next = next->getNext(ignoreRemoved);
        while (next->isDead());
    }
    return next;
}

Player *Player::getLastAlive(int n, bool ignoreRemoved) const
{
    return getNextAlive(aliveCount(!ignoreRemoved) - n, ignoreRemoved);
}

QList<const Player *> Player::getFormation() const
{
    QList<const Player *> teammates;
    teammates << this;
    int n = aliveCount(false);
    int num = n;
    for (int i = 1; i < n; ++i) {
        Player *target = getNextAlive(i);
        if (getKingdom() == target->getKingdom())
            teammates << target;
        else {
            num = i;
            break;
        }
    }

    n -= num;
    for (int i = 1; i < n; ++i) {
        Player *target = getLastAlive(i);
        if (getKingdom() == target->getKingdom())
            teammates << target;
        else break;
    }

    return teammates;
}
