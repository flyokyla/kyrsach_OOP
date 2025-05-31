#include "settings.h"
#include <QSettings>

QString Settings::language()
{
    QSettings settings;
    return settings.value("language", "en").toString();
}

void Settings::saveLanguage(const QString &lang)
{
    QSettings settings;
    settings.setValue("language", lang);
}

void Settings::saveWindowGeometry(const QByteArray &geometry)
{
    QSettings settings;
    settings.setValue("geometry", geometry);
}

QByteArray Settings::windowGeometry()
{
    QSettings settings;
    return settings.value("geometry").toByteArray();
}

void Settings::saveWindowState(const QByteArray &state)
{
    QSettings settings;
    settings.setValue("windowState", state);
}

QByteArray Settings::windowState()
{
    QSettings settings;
    return settings.value("windowState").toByteArray();
}
