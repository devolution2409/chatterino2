#pragma once

#include "common/SignalVector.hpp"
#include "controllers/highlights/HighlightPhrase.hpp"
#include "messages/Message.hpp"
#include "singletons/Settings.hpp"

namespace chatterino {

class HighlightModel;

class HighlightController
{
public:
    HighlightController();

    void initialize();

    UnsortedSignalVector<HighlightPhrase> phrases;

    HighlightModel *createModel(QObject *parent);

    void addHighlight(const MessagePtr &msg);

private:
    bool initialized = false;

    ChatterinoSetting<std::vector<HighlightPhrase>> highlightsSetting = {
        "/highlighting/highlights"};
};

}  // namespace chatterino
