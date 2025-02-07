#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/CCScene.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include <Geode/modify/CCNode.hpp>
#include "Manager.hpp"
#include "Utils.hpp"

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {

	struct Fields {
		CCNode* m_uiNode;
		CCNode* m_rotatedMenuContainer;
		CCNode* m_container;
		float m_scaleFactor = 1.f;
		CCRenderTexture* m_renderTexture;
		CCSprite* m_renderTo;
		CCLayerColor* m_blackOverlay;
		CCSize m_oldDesignResolution;
		CCSize m_newDesignResolution;
		CCSize m_originalScreenScale;
		CCSize m_newScreenScale;
		Manager* manager = Manager::getSharedInstance();
		float m_degrees = 90.f;
		bool m_initialized = false;
		bool m_skipZOrder = true;
		~Fields() {
			if (m_renderTexture) m_renderTexture->release();
		}
	};

	void applyWinSize() {
		if(m_fields->m_newDesignResolution.width != 0 && m_fields->m_newDesignResolution.height != 0) {
			auto view = cocos2d::CCEGLView::get();
			
			cocos2d::CCDirector::get()->m_obWinSizeInPoints = m_fields->m_newDesignResolution;
			view->setDesignResolutionSize(m_fields->m_newDesignResolution.width, m_fields->m_newDesignResolution.height, ResolutionPolicy::kResolutionExactFit);
			view->m_fScaleX = CCDirector::get()->getContentScaleFactor() * m_fields->m_newScreenScale.width;
			view->m_fScaleY = CCDirector::get()->getContentScaleFactor() * m_fields->m_newScreenScale.height;
		}
	}

	void restoreWinSize() {
		if(m_fields->m_oldDesignResolution.width != 0 && m_fields->m_oldDesignResolution.height != 0) {
			auto view = cocos2d::CCEGLView::get();

			cocos2d::CCDirector::get()->m_obWinSizeInPoints = m_fields->m_oldDesignResolution;
			view->setDesignResolutionSize(m_fields->m_oldDesignResolution.width, m_fields->m_oldDesignResolution.height, ResolutionPolicy::kResolutionExactFit);
			view->m_fScaleX = m_fields->m_originalScreenScale.width;
			view->m_fScaleY = m_fields->m_originalScreenScale.height;
		}
	}

	void onEnterTransitionDidFinish() {

		PlayLayer::onEnterTransitionDidFinish();

		if (!Utils::modEnabled()) return;

		CCSize winSize = CCDirector::get()->getWinSize();
		m_fields->m_container = CCNode::create();
		m_fields->m_container->setZOrder(100000);
		m_fields->m_container->setAnchorPoint({0.5f, 0.5f});
		m_fields->m_container->setContentSize(winSize);
		m_fields->m_container->setPosition(winSize/2);

		m_fields->m_rotatedMenuContainer = CCNode::create();
		m_fields->m_rotatedMenuContainer->setAnchorPoint({0.5f, 0.5f});
		m_fields->m_rotatedMenuContainer->setContentSize({winSize.height, winSize.width});
		m_fields->m_rotatedMenuContainer->setPosition(winSize/2);

		m_fields->m_blackOverlay = CCLayerColor::create({0, 0, 0, 255});
		m_fields->m_blackOverlay->setZOrder(0);
		m_fields->m_blackOverlay->setContentSize(winSize);

		m_fields->m_renderTexture = CCRenderTexture::create(winSize.width, winSize.height);
		m_fields->m_renderTexture->retain();
		m_fields->m_renderTo = CCSprite::createWithTexture(m_fields->m_renderTexture->getSprite()->getTexture());
		m_fields->m_renderTo->setFlipY(true);
		m_fields->m_renderTo->setZOrder(1);

		if (Utils::getBool("flipOrientation")) {
			m_fields->m_degrees = 270.f;
			m_fields->m_container->setRotation(180.f);
		}

		m_fields->m_renderTo->setRotation(90);
		m_fields->m_renderTo->setPosition(winSize/2);
		float scale = winSize.height / m_fields->m_renderTo->getContentWidth();
		m_fields->m_renderTo->setScale(scale);

		m_fields->m_uiNode = CCNode::create();
		m_fields->m_uiNode->setContentSize(winSize);
		m_fields->m_uiNode->setPosition(winSize/2);
		m_fields->m_uiNode->setAnchorPoint({0.5f, 0.5f});
		m_fields->m_uiNode->setZOrder(2);

		if (Utils::getBool("tokTikUI")) {
			CCNode* footer = createFooter();
			CCNode* actions = createActions(footer);
			CCNode* description = createDescLabel(footer);
			CCNode* username = createUsernameLabel(description);
			CCNode* icon = createSimplePlayer(actions);
			CCNode* forYou = createForYou();
			CCNode* search = createSearch(forYou);
			CCNode* live = createLive(forYou);
			CCNode* vibingCube = createVibingCube(actions);

			m_fields->m_uiNode->addChild(footer);
			m_fields->m_uiNode->addChild(actions);
			m_fields->m_uiNode->addChild(description);
			m_fields->m_uiNode->addChild(username);
			m_fields->m_uiNode->addChild(icon);
			m_fields->m_uiNode->addChild(forYou);
			m_fields->m_uiNode->addChild(search);
			m_fields->m_uiNode->addChild(live);
			m_fields->m_uiNode->addChild(vibingCube);

			if (Utils::getBool("interactiveFooter")) {
				createInteractiveFooter(footer);
			}
		}

		m_fields->m_container->addChild(m_fields->m_uiNode);
		m_fields->m_container->addChild(m_fields->m_renderTo);
		m_fields->m_container->addChild(m_fields->m_blackOverlay);
		m_fields->m_container->addChild(m_fields->m_rotatedMenuContainer);

		CCScene* currentScene = CCDirector::get()->m_pNextScene;
		currentScene->addChild(m_fields->m_container);

		auto view = cocos2d::CCEGLView::get();

		m_fields->m_oldDesignResolution = view->getDesignResolutionSize();
		float aspectRatio = winSize.width / winSize.height;
		m_fields->m_newDesignResolution = cocos2d::CCSize(roundf(320.f * aspectRatio), 320.f);

		m_fields->m_originalScreenScale = cocos2d::CCSize(view->m_fScaleX, view->m_fScaleY);
		m_fields->m_newScreenScale = cocos2d::CCSize(winSize.width / m_fields->m_newDesignResolution.width, winSize.height / m_fields->m_newDesignResolution.height);

		if(m_fields->m_oldDesignResolution != m_fields->m_newDesignResolution) applyWinSize();

		m_fields->m_renderTo->scheduleUpdate();
		m_fields->m_renderTo->schedule(schedule_selector(MyPlayLayer::updateRender));

		setVisible(false);

		m_fields->m_initialized = true;
	}

	void updateRender(float p0) {

		MyPlayLayer* mpl = static_cast<MyPlayLayer*>(PlayLayer::get());

		if (!mpl->m_fields->m_renderTexture) return;
		if (!mpl->m_fields->m_renderTo) return;
		
		mpl->m_fields->m_renderTexture->beginWithClear(0.0f, 0.0f, 0.0f, 1.0f);
		
		mpl->m_fields->m_container->setVisible(false);

		CCScene* currentScene = CCDirector::get()->m_pRunningScene;
		mpl->setVisible(true);
		currentScene->visit();
		mpl->setVisible(false);

		mpl->m_fields->m_container->setVisible(true);
		mpl->m_fields->m_renderTexture->end();
		mpl->m_fields->m_renderTo->setTexture(mpl->m_fields->m_renderTexture->getSprite()->getTexture());
	}

	void createInteractiveFooter(CCNode* footer) {
		CCMenu* interactiveFooter = CCMenu::create();

		RowLayout* rowLayout = RowLayout::create();
		rowLayout->setAutoScale(true);
		rowLayout->setAxisAlignment(AxisAlignment::Center);
		rowLayout->setGap(5.0f);

		interactiveFooter->setLayout(rowLayout);
		
		CCSprite* homeTabButton = CCSprite::create("square.png"_spr);
		CCMenuItemSpriteExtra* homeTab = CCMenuItemSpriteExtra::create(homeTabButton, footer, menu_selector(MyPlayLayer::exitPlayLayer));
		homeTab->setID("home-tab"_spr);
		homeTab->setOpacity(0);
		homeTab->setTag(1);

		interactiveFooter->addChild(homeTab);

		CCSprite* friendsTabButton = CCSprite::create("square.png"_spr);
		CCMenuItemSpriteExtra* friendsTab = CCMenuItemSpriteExtra::create(friendsTabButton, footer, menu_selector(MyPlayLayer::openFriends));
		friendsTab->setID("friends-tab"_spr);
		friendsTab->setOpacity(0);
		friendsTab->setTag(2);

		interactiveFooter->addChild(friendsTab);

		CCSprite* myLevelsTabButton = CCSprite::create("square.png"_spr);
		CCMenuItemSpriteExtra* myLevelsTab = CCMenuItemSpriteExtra::create(myLevelsTabButton, footer, menu_selector(MyPlayLayer::openMyLevels));
		myLevelsTab->setOpacity(0);
		myLevelsTab->setTag(3);
		myLevelsTab->setID("my-levels-tab"_spr);

		interactiveFooter->addChild(myLevelsTab);

		CCSprite* inboxTabButton = CCSprite::create("square.png"_spr);
		CCMenuItemSpriteExtra* inboxTab = CCMenuItemSpriteExtra::create(inboxTabButton, footer, menu_selector(MyPlayLayer::openMessages));
		inboxTab->setID("inbox-tab"_spr);
		inboxTab->setOpacity(0);
		inboxTab->setTag(4);

		interactiveFooter->addChild(inboxTab);
		interactiveFooter->updateLayout();

		CCSprite* profileTabButton = CCSprite::create("square.png"_spr);
		CCMenuItemSpriteExtra* profileTab = CCMenuItemSpriteExtra::create(profileTabButton, footer, menu_selector(MyPlayLayer::openProfile));
		profileTab->setID("profile-tab"_spr);
		profileTab->setOpacity(0);
		profileTab->setTag(5);
		interactiveFooter->addChild(profileTab);
		interactiveFooter->updateLayout();

		footer->addChild(interactiveFooter);
		interactiveFooter->setPosition({47.f, 15.f}); // these hardcoded values are fine because they are for a child of an existing node
		interactiveFooter->setScale(1.425f); // these hardcoded values are fine because they are for a child of an existing node
		interactiveFooter->setID("footer-menu"_spr);
	}

	CCSprite* createFooter() {
		CCSprite* footer = CCSprite::create("footer.png"_spr);
		CCSize winSize = CCDirector::get()->getWinSize();
		footer->setID("footer"_spr);
		footer->setRotation(90.f);
		footer->setAnchorPoint({0.5f, 0.f});
		footer->setPosition({0, winSize.height/2});

		m_fields->m_scaleFactor = winSize.height / footer->getContentWidth();
		footer->setScale(m_fields->m_scaleFactor);

		return footer;
	}

	CCSprite* createActions(CCNode* footer) {
		CCSprite* actions = CCSprite::create("actions.png"_spr);
		CCSize winSize = CCDirector::get()->getWinSize();
		actions->setID("actions"_spr);
		actions->setRotation(90.f);
		actions->setAnchorPoint({1.f, 0.f});
		float offset = 20.f;
		actions->setPosition({footer->getScaledContentHeight() + offset, offset});

		actions->setScale(m_fields->m_scaleFactor * 0.75);

		createLikesLabel(actions);
		createDownloadsLabel(actions);
		createCommentsLabel(actions);

		return actions;
	}

	CCLabelBMFont* createDescLabel(CCNode* footer) {
		std::string desc = "[This level's description does not follow TokTik's Community Guidelines, which help us foster an inclusive and authentic community and define the kind of content and behavior that's not allowed on our app.]";
		CCSize winSize = CCDirector::get()->getWinSize();
		
		if (m_level && !m_level->getUnpackedLevelDescription().empty()) desc = m_level->getUnpackedLevelDescription();

		float scaleMultiplier = 0.75f;

		CCLabelBMFont* descLabel = CCLabelBMFont::create(desc.c_str(), "tokTikFont.fnt"_spr, winSize.height * scaleMultiplier, kCCTextAlignmentLeft);
		descLabel->setID("desc"_spr);
		descLabel->setRotation(90.f);
		descLabel->setScale(scaleMultiplier/2);
		descLabel->setAnchorPoint({0, 0});

		float offset = 10.f;
		descLabel->setPosition({footer->getScaledContentHeight() + offset, winSize.height - offset});
		
		return descLabel;
	}

	CCLabelBMFont* createUsernameLabel(CCNode* description) {
		std::string username = "{User expunged by TokTik}";
		CCSize winSize = CCDirector::get()->getWinSize();

		if (m_level) {
			switch (m_level->m_levelType) {
				case GJLevelType::Local:
					username = "@RobTop";
					break;
				case GJLevelType::Editor:
					username = fmt::format("@{}", GameManager::get()->m_playerName);
					break;
				case GJLevelType::Saved:
					if (!m_level->m_creatorName.empty()) username = fmt::format("@{}", m_level->m_creatorName);
					break;
			}
		}

		float scaleMultiplier = 0.75f;

		CCLabelBMFont* authorLabel = CCLabelBMFont::create(username.c_str(), "tokTikFontBold.fnt"_spr, winSize.height * scaleMultiplier, kCCTextAlignmentLeft);
		authorLabel->setID("author"_spr);
		authorLabel->setScale(scaleMultiplier/2);
		authorLabel->setRotation(90.f);
		authorLabel->setAnchorPoint({0, 0});

		float offset = 10.f;

		authorLabel->setPosition({description->getScaledContentHeight() + description->getPosition().x + 5, winSize.height - offset});
		return authorLabel;
	}

	int whichIcon(GameManager* gm = GameManager::get()) {
		switch (gm->m_playerIconType) {
			case IconType::Ship:
				return gm->m_playerShip.value();
			case IconType::Ball:
				return gm->m_playerBall.value();
			case IconType::Ufo:
				return gm->m_playerBird.value();
			case IconType::Wave:
				return gm->m_playerDart.value();
			case IconType::Robot:
				return gm->m_playerRobot.value();
			case IconType::Spider:
				return gm->m_playerSpider.value();
			case IconType::Swing:
				return gm->m_playerSwing.value();
			case IconType::Jetpack:
				return gm->m_playerJetpack.value();
			default:
				return gm->m_playerFrame.value();
		}
	}

	SimplePlayer* createSimplePlayer(CCNode* actions) {
		GameManager* gm = GameManager::get();
		SimplePlayer* player = SimplePlayer::create(0);
		player->setID("player"_spr);

		player->updatePlayerFrame(whichIcon(), gm->m_playerIconType);
		player->setColor(gm->colorForIdx(gm->m_playerColor.value()));
		player->setSecondColor(gm->colorForIdx(gm->m_playerColor2.value()));
		player->enableCustomGlowColor(gm->colorForIdx(gm->m_playerGlowColor.value()));
		player->setGlowOutline(gm->colorForIdx(gm->m_playerGlowColor.value()));
		if (!gm->getPlayerGlow()) player->disableGlowOutline();

		player->setRotation(90.f);
		player->setContentSize(player->m_firstLayer->getContentSize());
		player->m_firstLayer->setPosition(player->m_firstLayer->getContentSize()/2);
		
		float scaleFactor = CCDirector::get()->getContentScaleFactor() / 4.f;
		
		player->setPosition({actions->getPositionX() + actions->getScaledContentHeight() - 3, actions->getPositionY()});
		player->setScale(actions->getScale() * (0.285 / scaleFactor));
		player->setAnchorPoint({1.f, 0.f});
		player->setZOrder(-1);
		return player;
	}

	CCSprite* createForYou() {
		CCSprite* forYou = CCSprite::create("followingAndFYP.png"_spr);
		CCSize winSize = CCDirector::get()->getWinSize();

		forYou->setID("for-you"_spr);
		forYou->setRotation(90.f);
		forYou->setAnchorPoint({0.5f, 1.f});

		float offset = 25.f;

		forYou->setPosition({winSize.width - offset, winSize.height/2});
		forYou->setScale(m_fields->m_scaleFactor * 0.8);

		return forYou;
	}

	CCSprite* createSearch(CCNode* forYou) {
		CCSprite* search = CCSprite::create("search.png"_spr);
		CCSize winSize = CCDirector::get()->getWinSize();

		search->setID("search"_spr);
		search->setRotation(90.f);
		search->setAnchorPoint({1.f, 1.f});

		float offset = 20.f;
		float heightOffset = 5.f;

		search->setPosition({forYou->getPositionX() + heightOffset, offset});
		search->setScale(forYou->getScale());
		return search;
	}

	CCSprite* createLive(CCNode* forYou) {
		CCSprite* live = CCSprite::create("live.png"_spr);
		CCSize winSize = CCDirector::get()->getWinSize();

		live->setID("live"_spr);
		live->setRotation(90.f);
		live->setAnchorPoint({0.f, 1.f});

		float offset = 20.f;
		float heightOffset = 5.f;

		live->setPosition({forYou->getPositionX() + heightOffset, winSize.height - offset});
		live->setScale(forYou->getScale());
		return live;
	}
	
	CCSprite* createVibingCube(CCNode* actions) {
		CCSprite* vibingCube = CCSprite::create("vibingCube.png"_spr);
		vibingCube->setID("vibing-cube"_spr);
		vibingCube->setRotation(90.f);
		vibingCube->setScale(actions->getScale() / 19);

		float offset = 6;

		vibingCube->setPosition({actions->getPositionX() + 5 + offset, actions->getPositionY() - offset});
		vibingCube->setAnchorPoint({1.f, 0.f});
		return vibingCube;
	}

	void createLikesLabel(CCNode* actions) {
		std::string likes = "0";
		if (m_level && m_level->m_levelType == GJLevelType::Saved) likes = utils::numToAbbreviatedString(m_level->m_likes);
		CCLabelBMFont* likesLabel = CCLabelBMFont::create(likes.c_str(), "tokTikFontBold.fnt"_spr);
		likesLabel->setScale(0.2f);
		likesLabel->setID("likes"_spr);
		likesLabel->setPosition({13.75, 63});

		actions->addChild(likesLabel);
	}
	void createCommentsLabel(CCNode* actions) {
		std::string comments = "0";
		if (m_level && m_level->m_levelType == GJLevelType::Saved) comments = utils::numToAbbreviatedString(abs((abs(m_level->m_downloads - m_level->m_likes) + 1) / 20) * 3);
		CCLabelBMFont* commentsLabel = CCLabelBMFont::create(comments.c_str(), "tokTikFontBold.fnt"_spr);
		commentsLabel->setScale(0.2f);
		commentsLabel->setID("comments"_spr);
		commentsLabel->setPosition({13.75, 44});

		actions->addChild(commentsLabel);
	}
	void createDownloadsLabel(CCNode* actions) {
		int downloads = 0;
		if (m_level && m_level->m_levelType == GJLevelType::Saved) downloads = m_level->m_downloads;
		CCLabelBMFont* shareLabel = CCLabelBMFont::create(utils::numToAbbreviatedString(downloads).c_str(), "tokTikFontBold.fnt"_spr);
		shareLabel->setScale(0.2f);
		shareLabel->setID("downloads"_spr);
		shareLabel->setPosition({13.75, 23});

		actions->addChild(shareLabel);
	}

	void onQuit() {
		m_fields->manager->senderTag = -1;
		PlayLayer::onQuit();
	}

	/*
		I don't think it is worth making the buttons work, as there are too many issues to count on 5 hands
		I have removed the setting, if you would like to look into resolving in the future, go for it :3
		I tried, I failed, is it worth it, probably not.
	*/

	void exitPlayLayer(CCObject* sender) {
		if (!Utils::modEnabled() || !Utils::getBool("tokTikUI") || !Utils::getBool("interactiveFooter") || !PlayLayer::get()) return;
		PauseLayer::create(false)->onQuit(nullptr);
		m_fields->manager->senderTag = -1;
	}

	void openFriends(CCObject* sender) {
		if (!Utils::modEnabled() || !Utils::getBool("tokTikUI") || !Utils::getBool("interactiveFooter") || !PlayLayer::get()) return;
		m_fields->m_skipZOrder = false;
		FriendsProfilePage::create(UserListType::Friends)->show();
		m_fields->m_skipZOrder = true;
		m_fields->manager->senderTag = sender->getTag();
	}

	void openMyLevels(CCObject* sender) {
		if (!Utils::modEnabled() || !Utils::getBool("tokTikUI") || !Utils::getBool("interactiveFooter") || !PlayLayer::get()) return;
		GameManager::get()->playMenuMusic();
		CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, LevelBrowserLayer::scene(GJSearchObject::create(SearchType::MyLevels))));
		m_fields->manager->senderTag = -1;
	}

	void openMessages(CCObject* sender) {
		if (!Utils::modEnabled() || !Utils::getBool("tokTikUI") || !Utils::getBool("interactiveFooter") || !PlayLayer::get()) return;
		m_fields->m_skipZOrder = false;
		MessagesProfilePage::create(false)->show();
		m_fields->m_skipZOrder = true;
		m_fields->manager->senderTag = sender->getTag();
	}

	void openProfile(CCObject* sender) {
		if (!Utils::modEnabled() || !Utils::getBool("tokTikUI") || !Utils::getBool("interactiveFooter") || !PlayLayer::get()) return;
		m_fields->m_skipZOrder = false;
		GameManager::get()->m_menuLayer->onMyProfile(nullptr);
		m_fields->m_skipZOrder = true;
		m_fields->manager->senderTag = sender->getTag();
	}
};

class $modify(MyCCScheduler, CCScheduler) {
	void update(float dt) {

		if (!Utils::modEnabled()) return CCScheduler::update(dt);

		if (PlayLayer* pl = PlayLayer::get()) {
			MyPlayLayer* npl = static_cast<MyPlayLayer*>(pl);
			npl->applyWinSize();
			CCScheduler::update(dt);
			npl->restoreWinSize();
			return;
		}
		CCScheduler::update(dt);
		return;
	}
};

class $modify(MyCCScene, CCScene) {

	int getHighestChildZ() {

		if (PlayLayer* pl = PlayLayer::get()) {
			MyPlayLayer* npl = static_cast<MyPlayLayer*>(pl);
			if (npl->m_fields->m_skipZOrder) {
				if (CCNode* container = npl->m_fields->m_container) {
					int origZ = container->getZOrder();
					container->setZOrder(-1);
					int ret = CCScene::getHighestChildZ();
					container->setZOrder(origZ);
					return ret;
				}
			}
		}

		return CCScene::getHighestChildZ();
	}
};

class $modify(MyCCTouchDispatcher, CCTouchDispatcher) {

	static void onModify(auto& self) {
		// silly doggo hook prio for android ball compat
		(void) self.setHookPriority("cocos2d::CCTouchDispatcher::touches", -999999999);
	}

	CCPoint scalePointAroundCenter(const CCPoint& point, const CCPoint& center, float scaleFactor) {

		CCPoint translatedPoint = point - center;
		translatedPoint.x *= scaleFactor;
		translatedPoint.y *= scaleFactor;
		CCPoint scaledPoint = translatedPoint + center;

		return scaledPoint;
	}

	void touches(CCSet* touches, CCEvent* event, unsigned int type) {
		
		auto* touch = static_cast<CCTouch*>(touches->anyObject());

		if (!touch) {
			return;
		}

		if (!Utils::modEnabled()) return CCTouchDispatcher::touches(touches, event, type);

		if (PlayLayer* pl = PlayLayer::get()) {
			
			MyPlayLayer* npl = static_cast<MyPlayLayer*>(pl);

			if (npl->m_fields->m_initialized) {
				npl->setVisible(true);
				CCSize winSize = CCDirector::get()->getWinSize();
				CCPoint center = winSize/2.f;

				CCPoint pos = touch->getLocation();

				if (!npl->m_fields->m_renderTo->boundingBox().containsPoint(pos)) return CCTouchDispatcher::touches(touches, event, type);

				float scale = npl->m_fields->m_renderTo->getScale();

				pos.y = winSize.height - pos.y;
				pos = scalePointAroundCenter(pos, center, 1/scale);
				
				CCPoint newPos = pos.rotateByAngle(center, CC_DEGREES_TO_RADIANS(npl->m_fields->m_degrees));
				newPos.y = winSize.height - newPos.y;
				newPos.x = winSize.width - newPos.x;

				touch->setTouchInfo(touch->getID(), newPos.x, newPos.y);
				CCTouchDispatcher::touches(touches, event, type);

				npl->setVisible(false);
				return;
			}
		}

		CCTouchDispatcher::touches(touches, event, type);
	}
};