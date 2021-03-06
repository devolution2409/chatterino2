#include "common/CompletionModel.hpp"

#include "Application.hpp"
#include "common/Common.hpp"
#include "controllers/commands/CommandController.hpp"
#include "debug/Log.hpp"
#include "singletons/Emotes.hpp"

#include <QtAlgorithms>

#include <utility>

namespace chatterino {

CompletionModel::CompletionModel(const QString &_channelName)
    : channelName(_channelName)
{
}

void CompletionModel::refresh()
{
    Log("[CompletionModel:{}] Refreshing...]", this->channelName);

    auto app = getApp();

    // User-specific: Twitch Emotes
    // TODO: Fix this so it properly updates with the proper api. oauth token needs proper scope
    for (const auto &m : app->emotes->twitch.emotes) {
        for (const auto &emoteName : m.second.emoteCodes) {
            // XXX: No way to discern between a twitch global emote and sub emote right now
            this->addString(emoteName, TaggedString::Type::TwitchGlobalEmote);
        }
    }

    // Global: BTTV Global Emotes
    std::vector<QString> &bttvGlobalEmoteCodes = app->emotes->bttv.globalEmoteCodes;
    for (const auto &m : bttvGlobalEmoteCodes) {
        this->addString(m, TaggedString::Type::BTTVGlobalEmote);
    }

    // Global: FFZ Global Emotes
    std::vector<QString> &ffzGlobalEmoteCodes = app->emotes->ffz.globalEmoteCodes;
    for (const auto &m : ffzGlobalEmoteCodes) {
        this->addString(m, TaggedString::Type::FFZGlobalEmote);
    }

    // Channel-specific: BTTV Channel Emotes
    std::vector<QString> &bttvChannelEmoteCodes =
        app->emotes->bttv.channelEmoteCodes[this->channelName];
    for (const auto &m : bttvChannelEmoteCodes) {
        this->addString(m, TaggedString::Type::BTTVChannelEmote);
    }

    // Channel-specific: FFZ Channel Emotes
    std::vector<QString> &ffzChannelEmoteCodes =
        app->emotes->ffz.channelEmoteCodes[this->channelName];
    for (const auto &m : ffzChannelEmoteCodes) {
        this->addString(m, TaggedString::Type::FFZChannelEmote);
    }

    // Global: Emojis
    const auto &emojiShortCodes = app->emotes->emojis.shortCodes;
    for (const auto &m : emojiShortCodes) {
        this->addString(":" + m + ":", TaggedString::Type::Emoji);
    }

    // Commands
    for (auto &command : app->commands->items.getVector()) {
        this->addString(command.name, TaggedString::Command);
    }

    for (auto &command : app->commands->getDefaultTwitchCommandList()) {
        this->addString(command, TaggedString::Command);
    }

    // Channel-specific: Usernames
    // fourtf: only works with twitch chat
    //    auto c = ChannelManager::getInstance().getTwitchChannel(this->channelName);
    //    auto usernames = c->getUsernamesForCompletions();
    //    for (const auto &name : usernames) {
    //        assert(!name.displayName.isEmpty());
    //        this->addString(name.displayName);
    //        this->addString('@' + name.displayName);

    //        if (!name.localizedName.isEmpty()) {
    //            this->addString(name.localizedName);
    //            this->addString('@' + name.localizedName);
    //        }
    //    }
}

void CompletionModel::addString(const QString &str, TaggedString::Type type)
{
    std::lock_guard<std::mutex> lock(this->emotesMutex);

    // Always add a space at the end of completions
    this->emotes.insert({str + " ", type});
}

void CompletionModel::addUser(const QString &username)
{
    auto add = [this](const QString &str) {
        auto ts = this->createUser(str + " ");
        // Always add a space at the end of completions
        std::pair<std::set<TaggedString>::iterator, bool> p = this->emotes.insert(ts);
        if (!p.second) {
            // No inseration was made, figure out if we need to replace the username.

            if (p.first->str > ts.str) {
                // Replace lowercase version of name with mixed-case version
                this->emotes.erase(p.first);
                auto result2 = this->emotes.insert(ts);
                assert(result2.second);
            } else {
                p.first->timeAdded = std::chrono::steady_clock::now();
            }
        }
    };

    add(username);
    add("@" + username);
}

void CompletionModel::ClearExpiredStrings()
{
    std::lock_guard<std::mutex> lock(this->emotesMutex);

    auto now = std::chrono::steady_clock::now();

    for (auto it = this->emotes.begin(); it != this->emotes.end();) {
        const auto &taggedString = *it;

        if (taggedString.HasExpired(now)) {
            // Log("String {} expired", taggedString.str);
            it = this->emotes.erase(it);
        } else {
            ++it;
        }
    }
}

}  // namespace chatterino
