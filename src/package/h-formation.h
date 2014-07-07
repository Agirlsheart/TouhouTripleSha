#ifndef _H_FORMATION_H
#define _H_FORMATION_H

#include "package.h"
#include "card.h"
#include "skill.h"

class ShangyiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ShangyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HFormationPackage: public Package {
    Q_OBJECT

public:
    HFormationPackage();
};

#endif
