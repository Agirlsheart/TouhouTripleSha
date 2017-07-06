#include "achievement.h"
#include "engine.h"

#include <QFile>

AchieveSkill::AchieveSkill(QString objectName)
    : TriggerSkill("#achievement_" + objectName)
{
    key = objectName;
    frequency = Compulsory;
    global = true;
}

int AchieveSkill::getPriority(TriggerEvent) const
{
    return 0;
}

void AchieveSkill::onGameOver(Room *, ServerPlayer *, QVariant &) const
{
    return;
}

#define ACCOUNT "account/docs/data/"

void AchieveSkill::gainAchievement(ServerPlayer *player, Room *room) const
{
    int uid = player->userId();
    if (uid == -1)
        return;
    QStringList list = room->getAchievementData(player, "finished").toString().split("|");
    list << key;
    room->setAchievementData(player, "finished", list.join("|"));
    QStringList translations = getAchievementTranslations(key);
    if (translations.length() == 2) {
        LogMessage log;
        log.type = "#GainAchievement";
        log.arg = player->screenName();
        log.arg2 = translations[0];
        room->sendLog(log);
        log.type = "#AchievementDescription";
        log.arg = translations[1];
        room->sendLog(log);
    }
    QString location = QString(ACCOUNT"%1_achievement.csv").arg(uid);
    QFile file(location);
    file.open(QIODevice::ReadWrite);
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QStringList lines = stream.readAll().split("\n", QString::SkipEmptyParts);
    stream.seek(0);
    bool modified = false;
    foreach (QString line, lines) {
        QStringList _line = line.split(",");
        if (_line.length() != 4) {
            stream << line << "\n";
        } else if (_line[0] == key) {
            int num = _line[1].toInt() + 1;
            QString date = _line[2];
            if (date == "1990-01-01") {
                QDateTime time = QDateTime::currentDateTime();
                date = time.toString("yyyy-MM-dd");
            }
            stream << QString("%1,%2,%3,%4").arg(key).arg(num).arg(date).arg(_line[3]) << "\n";
            modified = true;
        } else
            stream << line << "\n";
    }
    if (!modified) {
        stream.seek(file.size());
        QDateTime time = QDateTime::currentDateTime();
        stream << QString("%1,1,%2,-1").arg(key).arg(time.toString("yyyy-MM-dd")) << "\n";
    }
    stream.flush();
    file.close();
}

QStringList AchieveSkill::getAchievementTranslations(QString _key)
{
    QString location = QString(ACCOUNT"achievement.csv");
    QFile file(location);
    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList _line = line.split(",");
        if (_line.length() != 5)
            continue;
        if (_line[0] == _key) {
            stream.flush();
            file.close();
            QStringList lists;
            lists << _line[1] << _line[2];
            return lists;
        }
    }
    stream.flush();
    file.close();
    return QStringList();
}

class AchievementMain : public AchieveSkill
{
public:
    AchievementMain()
        : AchieveSkill("main")
    {
        events << Death;
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *player, QVariant &data, ServerPlayer *&) const
    {
        DeathStruct death = data.value<DeathStruct>();
        if (death.who == player)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        DeathStruct death = data.value<DeathStruct>();
        if (player->isOffline())
            player->setProperty("run", true);
        if (death.damage && death.damage->from) {
            QStringList killed_roles = room->getAchievementData(death.damage->from, "killed_roles").toStringList();
            killed_roles << player->getRole();
            room->setAchievementData(death.damage->from, "killed_roles", QVariant::fromValue(killed_roles));
        }
        return false;
    }

    virtual void onGameOver(Room *room, ServerPlayer *player, QVariant &data) const
    {
        DeathStruct death = data.value<DeathStruct>();
        if (player->isOffline())
            player->setProperty("run", true);
        if (death.damage && death.damage->from) {
            QStringList killed_roles = room->getAchievementData(death.damage->from, "killed_roles").toStringList();
            killed_roles << player->getRole();
            room->setAchievementData(death.damage->from, "killed_roles", QVariant::fromValue(killed_roles));
        }
    }
};

class WenGongWuGong : public AchieveSkill
{
public:
    WenGongWuGong()
        : AchieveSkill("wengongwugong")
    {
        events << DamageDone << EventPhaseProceeding;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *, ServerPlayer *player, QVariant &data,
                                    ServerPlayer *&ask_who) const
    {
        if (triggerEvent == DamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from) {
                ask_who = damage.from;
                return QStringList(objectName());
            }
        } else if (triggerEvent == EventPhaseProceeding) {
            if (player->getPhase() == Player::Play)
                return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data,
                        ServerPlayer *ask_who) const
    {
        if (triggerEvent == DamageDone) {
            room->addAchievementData(ask_who, "wu", data.value<DamageStruct>().damage);
        } else if (triggerEvent == EventPhaseProceeding) {
            room->addAchievementData(player, "wen");
        }
        return false;
    }
};

class AchievementRecord : public AchieveSkill
{
public:
    AchievementRecord()
        : AchieveSkill("record")
    {
        events << GameOverJudge << BeforeGameOver;
        // only trigger BeforeGameOver when standoff/draw/surrender
        // other achieve skill cannot set to GameOverJudge in events, but set the trigger in AchieveSkill::onGameOver
    }

    virtual QStringList triggerable(TriggerEvent e, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *&) const
    {
        if (e == GameOverJudge) {
            QString winner = room->getWinner(player);
            if (!winner.isEmpty())
                return QStringList(objectName());
        } else if (e == BeforeGameOver)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent e, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (e == GameOverJudge) {
            const Package *package = Sanguosha->getPackage("achievement");
            if (package) {
                QList<const Skill *> skills = package->getSkills();
                foreach (const Skill *s, skills) {
                    if (s->inherits("AchieveSkill")) {
                        const AchieveSkill *as = qobject_cast<const AchieveSkill *>(s);
                        as->onGameOver(room, player, data);
                    }
                }
            }
        }
        QStringList winners;
        QStringList alive_roles;
        if (e == GameOverJudge) {
            QString winner = room->getWinner(player);
            winners = winner.split("+");
            alive_roles = room->aliveRoles(player);
        } else if (e == BeforeGameOver) {
            QString winner = data.toString();
            winners = winner.split("+");
            alive_roles = room->aliveRoles();
        }
        foreach (ServerPlayer *p, room->getPlayers())
            addPlayerRecord(room, p, winners, alive_roles);
        return false;
    }

    int getExp(Room *room, ServerPlayer *player, QStringList winners, QStringList alive_roles) const
    {
        int exp = 5;
        QStringList roles = room->getAchievementData(player, "killed_roles").toStringList();
        switch (player->getRoleEnum()) {
        case Player::Lord:
            if (winners.contains("lord"))
                exp += 4 + 2 * alive_roles.count("loyalist");
            else if (alive_roles.length() == 1 && alive_roles.first() == "renegade")
                exp += 1;
            exp += roles.count("rebel") + roles.count("renegade");
            break;
        case Player::Loyalist:
            if (winners.contains("loyalist"))
                exp += 5 + 2 * alive_roles.count("loyalist");
            exp += roles.count("rebel") + roles.count("renegade");
            break;
        case Player::Rebel:
            if (winners.contains("rebel"))
                exp += 3 * alive_roles.count("rebel");
            exp += roles.count("loyalist") + 2 * roles.count("lord");
            break;
        case Player::Renegade:
            if (player->property("1v1").toBool())
                exp += 8;
            if (winners.contains("rebel") && player->isAlive())
                exp += 1;
            if (winners.contains(player->objectName()))
                exp += 20;
            break;
        }
        return exp;
    }

    void addPlayerRecord(Room *room, ServerPlayer *player, QStringList winners, QStringList alive_roles) const
    {
        QString role = player->getRole();
        bool is_escape = player->property("run").toBool();
        if (player->isAlive())
            is_escape = player->isOffline();
        bool is_draw = winners.contains(".");
        bool is_win = winners.contains(player->getRole()) || winners.contains(player->objectName());
        bool is_alive = player->isAlive();
        int exp = 0;
        if (!is_escape) {
            if (is_draw) {
                if (is_alive)
                    exp += 1;
            } else
                exp = getExp(room, player, winners, alive_roles);
        }
        int wen = room->getAchievementData(player, "wen").toInt();
        int wu = room->getAchievementData(player, "wu").toInt();
        QString _finished_achieve = room->getAchievementData(player, "finished").toString();
        QStringList finished_achieve = _finished_achieve.split("|");
        QStringList translated_achieve;
        foreach (QString _key, finished_achieve) {
            QStringList trans = getAchievementTranslations(_key);
            if (trans.isEmpty())
                continue;
            translated_achieve << trans[0];
        }
        if (translated_achieve.isEmpty())
            translated_achieve << AchieveSkill::tr("None");
        room->setPlayerProperty(player, "gain", QString("%1,%2,%3,%4").arg(exp).arg(wen).arg(wu).arg(_finished_achieve));
        int uid = player->userId();
        if (uid == -1)
            return;
        updatePlayerData(uid, exp, wen, wu);
        QStringList line;
        line << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
        line << Sanguosha->translate(player->getGeneralName());
        line << Sanguosha->getModeName(room->getMode());
        line << Sanguosha->translate(role);
        line << QString::number(player->getMark("Global_TurnCount"));
        line << (is_alive ? AchieveSkill::tr("Alive") : AchieveSkill::tr("Dead"));
        line << (is_escape ? AchieveSkill::tr("Escape")
                           : (is_draw ? AchieveSkill::tr("Standoff")
                                      : (is_win ? AchieveSkill::tr("Victory") : AchieveSkill::tr("Failure"))));
        line << QString::number(exp);
        line << QString::number(wen);
        line << QString::number(wu);
        line << translated_achieve.join(" ");
        QString location = QString(ACCOUNT"%1_records.csv").arg(uid);
        QFile file(location);
        if (!file.open(QIODevice::ReadWrite))
            return;
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream.seek(file.size());
        stream << line.join(",") << "\n";
        stream.flush();
        file.close();
    }

    void updatePlayerData(int uid, int exp, int wen, int wu) const
    {
        QString location = QString(ACCOUNT"accounts.csv");
        QFile file(location);
        if (!file.open(QIODevice::ReadWrite))
            return;
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        QStringList lines = stream.readAll().split("\n", QString::SkipEmptyParts);
        stream.seek(0);
        foreach (QString line, lines) {
            QStringList _line = line.split(",");
            if (_line.length() != 6) {
                stream << line << "\n";
            } else if (_line[0] == QString::number(uid)) {
                _line[3] = QString::number(_line[3].toInt() + exp);
                _line[4] = QString::number(_line[4].toInt() + wen);
                _line[5] = QString::number(_line[5].toInt() + wu);
                stream << _line.join(",") << "\n";
            } else
                stream << line << "\n";
        }
        stream.flush();
        file.close();
    }
};

class HFLY : public AchieveSkill
{
public:
    HFLY()
        : AchieveSkill("hfly")
    {
        events << PreDamageDone << CardFinished;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data,
                                    ServerPlayer *&) const
    {
        if (triggerEvent == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            QVariant value = room->getAchievementData(player, key);
            QVariantList v_list = value.toList();
            QMap<QString, QStringList> map;
            for (int i = 0; i < v_list.length(); ++i) {
                QString s = v_list[i].toString();
                ++i;
                map[s] = v_list[i].toStringList();
            }
            if (!map.value(use.card->toString(), QStringList()).isEmpty())
                map.remove(use.card->toString());
            v_list.clear();
            foreach (QString _key, map.keys()) {
                v_list << _key;
                v_list << QVariant::fromValue(map[_key]);
            }
            room->setAchievementData(player, key, QVariant::fromValue(v_list));
        } else {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from && damage.card && damage.card->isKindOf("BurningCamps")) {
                QVariant value = room->getAchievementData(damage.from, key);
                QVariantList v_list = value.toList();
                QMap<QString, QStringList> map;
                for (int i = 0; i < v_list.length(); ++i) {
                    QString s = v_list[i].toString();
                    ++i;
                    map[s] = v_list[i].toStringList();
                }
                if (map.value(damage.card->toString(), QStringList()).isEmpty()) {
                    map[damage.card->toString()] = QStringList();
                    map[damage.card->toString()] << player->objectName();
                } else {
                    if (!map[damage.card->toString()].contains(player->objectName()))
                        map[damage.card->toString()] << player->objectName();
                }
                v_list.clear();
                foreach (QString _key, map.keys()) {
                    v_list << _key;
                    v_list << QVariant::fromValue(map[_key]);
                }
                room->setAchievementData(damage.from, key, QVariant::fromValue(v_list));
                if (map[damage.card->toString()].length() == 6)
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        gainAchievement(damage.from, room);
        return false;
    }
};

class XQWBJFY : public AchieveSkill
{
public:
    XQWBJFY()
        : AchieveSkill("xqwbjfy")
    {
        events << ChoiceMade << CardFinished;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *&) const
    {
        if (triggerEvent == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Drowning"))
                room->setAchievementData(player, key, 0);
        } else {
            QStringList args = data.toString().split(":");
            if (args[0] == "cardChosen") {
                if (args[1] == "drowning")
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        int value = room->getAchievementData(player, key).toInt();
        ++value;
        room->setAchievementData(player, key, value);
        if (value == 8)
            gainAchievement(player, room);
        return false;
    }
};

class TSQB : public AchieveSkill
{
public:
    TSQB()
        : AchieveSkill("tsqb")
    {
        events << CardUsed << EventPhaseChanging;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *, QVariant &data,
                                    ServerPlayer *&) const
    {
        if (triggerEvent == EventPhaseChanging) {
            if (data.value<PhaseChangeStruct>().to == Player::NotActive) {
                foreach (ServerPlayer *p, room->getAlivePlayers())
                    room->setAchievementData(p, key, 0);
            }
            return QStringList();
        }
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash"))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        room->addAchievementData(player, key, 1);
        if (room->getAchievementData(player, key) == 4)
            gainAchievement(player, room);
        return false;
    }
};

AchievementPackage::AchievementPackage()
    : Package("achievement", SpecialPack)
{
    skills << new AchievementMain << new WenGongWuGong << new AchievementRecord;
    skills << new HFLY << new XQWBJFY << new TSQB;
}

ADD_PACKAGE(Achievement)
