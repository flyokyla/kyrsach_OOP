#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

class Settings
{
public:
    static QString language();
    static void saveLanguage(const QString &lang);
    
    static void saveWindowGeometry(const QByteArray &geometry);
    static QByteArray windowGeometry();
    
    static void saveWindowState(const QByteArray &state);
    static QByteArray windowState();

private:
    Settings() = default;
};

#endif // SETTINGS_H
