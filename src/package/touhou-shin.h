#ifndef TOUHOUSHIN_H
#define TOUHOUSHIN_H

#include "package.h"
#include "card.h"

class TouhouShinPackage : public Package{
    Q_OBJECT

public:
    TouhouShinPackage();
};

class ThLuanshenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ThLuanshenCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ThLianyingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ThLianyingCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ThLiaoganCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ThLiaoganCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // TOUHOUSHIN_H