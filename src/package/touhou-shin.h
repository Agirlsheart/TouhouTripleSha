#ifndef TOUHOUSHIN_H
#define TOUHOUSHIN_H

#include "card.h"
#include "package.h"

class TouhouShinPackage : public Package
{
    Q_OBJECT

public:
    TouhouShinPackage();
};

class ThLuanshenCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ThLuanshenCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ThLianyingCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ThLianyingCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ThMumiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ThMumiCard();
};

class ThShenmiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ThShenmiCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

class ThMuyuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ThMuyuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ThNihuiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ThNihuiCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif // TOUHOUSHIN_H
