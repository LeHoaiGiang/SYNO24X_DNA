#ifndef AMIDITE_H
#define AMIDITE_H

#include <QDebug>
#include <QTime>
#include <QString>
#include "QScreen"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDataStream>
#include <QDir>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QAbstractTableModel>
#include "struct.h"
#include "qlineedit.h"
enum LinkageType { O, S };
struct CharacterLinkage {
    QChar character;
    LinkageType linkage;
};

class amidite
{
public:

    QString amiditePath;
    QString sequence_floatting_1;
    QString sequence_floatting_2;

    QLineEdit * bottleNameInputs[6];
    QStringList bottleNamesListFull;
    QStringList bottleNamesListAmidite;
    const QStringList bottleNamesFirstList = {"A", "T", "G", "C"}; // 4 AMIDITE không thay đổi
    QStringList bottleNamesList = {"FAM", "HEX", "CY5", "TET", "ROX", "BHQ"}; // floating amidte sẽ bị edit bởi người dùng
    const QStringList bottleNamesLastList = {"Activator", "TCA", "WASH", "OXidation 1", "OXidation 2", "CAPB", "CAPA"};// hóa chất chung cũng không thay đổi
    //QString string_sequence[MAX_WELL_AMIDITE];
    amidite();
    void setpath(QString Path);
    void saveAmiditeFloatting(const QString& amidite_f1, const QString& amidite_f2);
    void readAmiditeFloatting(QString& amidite_f1, QString& amidite_f2);
    void amiditeSyno24XGetSequence(global_var_t* global_var);
    quint8 char2quint8_Amidite(char DataIn);
    QString transformString(const QString &input, const QString &editTextFloating_1, const QString &editTextFloating_2);
    QList<CharacterLinkage> convertToLinkages(const QString &input);
    void printLinkages(const QList<CharacterLinkage> &linkages);
    QString  reverseStringWithParentheses(const QString& input);
    void determineCharactersAndLinkages(const QString& sequence, QVector<QChar>& characters, QVector<QChar>& linkages);
    QString transformString_Ox(const QString &input, const QString &editTextFloating_1, const QString &editTextFloating_2) ;
    QString transformString_OxFloatting(const QString &input, const QStringList &bottleNames);
    QString transformString_SequenceFloatting(const QString &input, const QStringList &bottleNames);
    bool validateInput(const QString &input, const QStringList &bottleNamesList, QString &errorMessage);
    bool validateAllWells(global_var_t* global_var);
    void saveToJson();
    void loadFromJson();
};

#endif // AMIDITE_H
