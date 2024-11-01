#include <Geode/modify/ProfilePage.hpp>
#include "Manager.hpp"
#include "Utils.hpp"

#define PREFERRED_HOOK_PRIO (-2123456789)

using namespace geode::prelude;

class $modify(MyProfilePage, ProfilePage) {
	static void onModify(auto &self) {
		(void) self.setHookPriority("ProfilePage::onMyLevels", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("ProfilePage::onMyLists", PREFERRED_HOOK_PRIO);
	}
	struct Fields {
		Manager* manager = Manager::getSharedInstance();
	};
	void onMyLevels(cocos2d::CCObject* sender) {
		if (!Utils::modEnabled() || !Utils::getBool("tokTikUI") || !Utils::getBool("footerMenu") || !PlayLayer::get() || m_fields->manager->senderTag == -1) return ProfilePage::onMyLevels(sender);
		Utils::showGuardrailCode();
	}
	void onMyLists(cocos2d::CCObject* sender) {
		if (!Utils::modEnabled() || !Utils::getBool("tokTikUI") || !Utils::getBool("footerMenu") || !PlayLayer::get() || m_fields->manager->senderTag == -1) return ProfilePage::onMyLists(sender);
		Utils::showGuardrailCode();
	}
};