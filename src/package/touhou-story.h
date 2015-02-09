#ifndef TOUHOUSTORY_H
#define TOUHOUSTORY_H

#include "package.h"
#include "card.h"

class HulaopassPackage : public Package{
    Q_OBJECT

public:
    HulaopassPackage();
};

class ThChayinCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ThChayinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#include <QDialog>
#include <QVBoxLayout>
#include <QCommandLinkButton>
#include <QButtonGroup>
class ThShenmieDialog: public QDialog {
    Q_OBJECT

public:
    static ThShenmieDialog *getInstance();

public slots:
    void popup();
    void selectCard(QAbstractButton *button);

private:
    explicit ThShenmieDialog();

    QButtonGroup *group;
    QVBoxLayout *button_layout;

    QHash<QString, const Card *> map;

signals:
    void onButtonClick();
};

class ThShenmieCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE ThShenmieCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
};

#endif // TOUHOUSTORY_H
