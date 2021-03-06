#include "singletons/Emotes.hpp"

#include "Application.hpp"
#include "controllers/accounts/AccountController.hpp"

namespace chatterino {

void Emotes::initialize()
{
    getApp()->accounts->twitch.currentUserChanged.connect([this] {
        auto currentUser = getApp()->accounts->twitch.getCurrent();
        assert(currentUser);
        this->twitch.refresh(currentUser);
    });

    this->emojis.load();
    this->bttv.loadGlobalEmotes();
    this->ffz.loadGlobalEmotes();

    this->gifTimer.initialize();
}

bool Emotes::isIgnoredEmote(const QString &)
{
    return false;
}

}  // namespace chatterino
