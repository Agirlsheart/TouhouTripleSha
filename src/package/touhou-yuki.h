#ifndef TOUHOUYUKI_H
#define TOUHOUYUKI_H

#include "package.h"
#include "card.h"
#include "skill.h"

class TouhouYukiPackage: public Package {
    Q_OBJECT

public:
    TouhouYukiPackage();
};

class ThMojiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ThMojiCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

class ThYuanqiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ThYuanqiCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ThDunjiaCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ThDunjiaCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual const Card *validate(CardUseStruct &card_use) const;
};

class ThChouceCard: public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ThChouceCard();

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *player) const;
};

class ThBingpuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ThBingpuCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ThDongmoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ThDongmoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ThHuanfaCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ThHuanfaCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ThKujieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ThKujieCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ThChuanshangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ThChuanshangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ThLingdieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ThLingdieCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ThFuyueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ThFuyueCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif
