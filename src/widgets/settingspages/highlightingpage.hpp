#pragma once

#include "widgets/settingspages/settingspage.hpp"

#include <QAbstractTableModel>
#include <QTimer>

class QPushButton;
class QListWidget;

namespace chatterino {
namespace widgets {
namespace settingspages {

class HighlightingPage : public SettingsPage
{
public:
    HighlightingPage();
};

}  // namespace settingspages
}  // namespace widgets
}  // namespace chatterino
