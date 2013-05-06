#include "aux-skills.h"
#include "client.h"
#include "carditem.h"
#include "standard.h"
#include "clientplayer.h"
#include "standard-generals.h"
#include "engine.h"

DiscardSkill::DiscardSkill()
    :ViewAsSkill("discard"), card(new DummyCard),
      num(0), include_equip(false), is_discard(true)
{
    card->setParent(this);
}

void DiscardSkill::setNum(int num){
    this->num = num;
}

void DiscardSkill::setMinNum(int minnum){
    this->minnum = minnum;
}

void DiscardSkill::setIncludeEquip(bool include_equip){
    this->include_equip = include_equip;
}

void DiscardSkill::setIsDiscard(bool is_discard) {
    this->is_discard = is_discard;
}

bool DiscardSkill::viewFilter(const QList<const Card *> &selected, const Card* card) const{
    if(selected.length() >= num)
        return false;

    if(!include_equip && card->isEquipped())
        return false;

    if (is_discard && Self->isCardLimited(card, Card::MethodDiscard))
        return false;

    return true;
}

const Card* DiscardSkill::viewAs(const QList<const Card*> &cards) const{
    if(cards.length() >= minnum){
        card->clearSubcards();
        card->addSubcards(cards);
        return card;
    }else
        return NULL;
}

// -------------------------------------------

ResponseSkill::ResponseSkill()
    :OneCardViewAsSkill("response-skill")
{

}

void ResponseSkill::setPattern(const QString &pattern){
    this->pattern = Sanguosha->getPattern(pattern);
}

void ResponseSkill::setRequire(const Card::HandlingMethod require) {
    this->require = require;
}

bool ResponseSkill::matchPattern(const Player *player, const Card *card) const{
    if (require != Card::MethodNone && player->isCardLimited(card, require))
        return false;

    return pattern && pattern->match(player, card);
}

bool ResponseSkill::viewFilter(const Card *card) const{
    return matchPattern(Self, card);
}

const Card *ResponseSkill::viewAs(const Card *originalCard) const{
    return originalCard;
}

// -------------------------------------------

ShowOrPindianSkill::ShowOrPindianSkill()
{
    setObjectName("showorpindian-skill");
}

bool ShowOrPindianSkill::matchPattern(const Player *player, const Card *card) const{
    return pattern && pattern->match(player, card);
}

// -------------------------------------------

FreeDiscardSkill::FreeDiscardSkill(QObject *parent)
    :ViewAsSkill("free-discard")
{
    setParent(parent);
    card = new DummyCard;
}

bool FreeDiscardSkill::isEnabledAtPlay(const Player *) const{
    return true;
}

bool FreeDiscardSkill::viewFilter(const QList<const Card *> &, const Card *) const{
    return true;
}

const Card *FreeDiscardSkill::viewAs(const QList<const Card *> &cards) const{
    if(!cards.isEmpty()){

        card->clearSubcards();
        card->addSubcards(cards);

        return card;
    }else
        return NULL;
}

// -------------------------------------------

YumengViewAsSkill::YumengViewAsSkill()
    :ViewAsSkill("yumeng")
{
    card = new FunuanCard;
}

void YumengViewAsSkill::setCards(const QString &card_str)
{
    QStringList cards = card_str.split("+");
    ids = Card::StringsToIds(cards);
}

void YumengViewAsSkill::setMaxNum(int max_num) {
    this->max_num = max_num;
}

bool YumengViewAsSkill::viewFilter(const QList<const Card *> &selected, const Card* card) const
{
    return ids.contains(card->getId()) && selected.length() < max_num;
}

const Card *YumengViewAsSkill::viewAs(const QList<const Card *> &cards) const
{
    if(cards.isEmpty())
        return NULL;

    card->clearSubcards();
    card->addSubcards(cards);

    return card;
}

// ------------------------------------------------

class ChoosePlayerCard: public DummyCard{
public:
    ChoosePlayerCard(){
        target_fixed = false;
    }

    void setPlayerNames(const QStringList &names){
        set = names.toSet();
    }

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
        return targets.isEmpty() && set.contains(to_select->objectName());
    }

private:
    QSet<QString> set;
};

ChoosePlayerSkill::ChoosePlayerSkill()
    :ZeroCardViewAsSkill("choose_player")
{
    card = new ChoosePlayerCard;
    card->setParent(this);
}

void ChoosePlayerSkill::setPlayerNames(const QStringList &names){
    card->setPlayerNames(names);
}

const Card *ChoosePlayerSkill::viewAs() const{
    return card;
}
