--缄魔：其他角色的出牌阶段开始时，若其手牌数不小于体力上限，你可令其选择一项：摸一张牌，且此阶段不能使用或打出【杀】；或此阶段使用【杀】时需弃置一张牌，否则此【杀】无效。
sgs.ai_skill_invoke.thjianmo = function(self, data)
	local target = data:toPlayer()
	if self:isEnemy(target) then
		return true
	end
	return false
end

sgs.ai_skill_choice.thjianmo = function(self, choices)
	if self:getCardsNum("Slash") < 1 then
		return "jian"
	end
	if self:getOverflow() < -1 then
		return "jian"
	end
	return "mo"
end

sgs.ai_skill_discard.thjianmo = function(self, discard_num, min_num, optional, include_equip)
	local ret = self:askForDiscard("", 1, 1, false, true)
	if #ret ~= 0 then
		if isCard("Peach", ret[1], self.player) then
			return {}
		else
			return ret
		end
	else
		return {}
	end
end

sgs.ai_choicemade_filter.skillInvoke.thjianmo = function(self, player, promptlist)
	if promptlist[#promptlist] == "yes" then
		local target = self.room:findPlayer(promptlist[#promptlist - 1])
		if target then
			sgs.updateIntention(player, target, 30)
		end
	end
end

--二重:觉醒技，若你回合外的两个连续的回合内，当前回合角色均未使用【杀】，且第二个回合的回合结束时，若你已受伤，你须减少1点体力上限，并获得技能“幻法”和“祝祭”。
--无

--春度：君主技，每当其他雪势力角色使用的红桃基本牌结算后置入弃牌堆时，你可弃置一张手牌获得之。
sgs.ai_skill_cardask["@thchundu"] = function(self, data, pattern)
	local card = data:toMoveOneTime().reason.m_extraData:toCard()
	if card then
		local h_cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(h_cards)
		if self:getKeepValue(card) > self:getKeepValue(h_cards[1]) then
			return "$" .. h_cards[1]
		end
	end
	return "."
end

--醉觞：你的回合内，所有角色可以将两张牌当【酒】使用（你的回合内，所有角色使用的【酒】不计入使用限制）。
local thzuishang_skill = {}
thzuishang_skill.name = "thzuishang"
table.insert(sgs.ai_skills, thzuishang_skill)
thzuishang_skill.getTurnUseCard = function(self, inclusive)
	local cards = self.player:getCards("he")
	for _, id in sgs.qlist(self.player:getPile("wooden_ox")) do
		cards:prepend(sgs.Sanguosha:getCard(id))
	end
	cards = sgs.QList2Table(cards)
	for _, acard in ipairs(cards) do
		if isCard("Analeptic", acard, self.player) then return end
	end
	self:sortByUseValue(cards)
	local newcards = {}
	local has_slash = false
	for _, card in ipairs(cards) do
		if self:getCardsNum("Slash") == 1 and isCard("Slash", card, self.player) then
			continue
		end
		if self:getCardsNum("Slash") == 2 and isCard("Slash", card, self.player) and has_slash then
			continue
		end
		if not isCard("Analeptic", card, self.player) and not isCard("Peach", card, self.player) and not (isCard("ExNihilo", card, self.player) and self.player:getPhase() == sgs.Player_Play) then
			if isCard("Slash", card, self.player) then
				has_slash = true
			end
			table.insert(newcards, card)
		end
	end
	if #newcards <= self.player:getHp() - 1 and self.player:getHp() <= 4 and self:needKongcheng()
		and not (self.player:hasSkill("ikshengtian") and self.player:getMark("@shengtian") == 0) then return end
	if #newcards < 2 then return end

	local card_id1 = newcards[1]:getEffectiveId()
	local card_id2 = newcards[2]:getEffectiveId()

	local card_str = ("analeptic:%s[%s:%s]=%d+%d"):format("thzuishang", "to_be_decided", 0, card_id1, card_id2)
	local analeptic = sgs.Card_Parse(card_str)
	return analeptic
end

function cardsView_thzuishang(self, player)
	local cards = player:getCards("he")
	for _, id in sgs.qlist(player:getPile("wooden_ox")) do
		cards:prepend(sgs.Sanguosha:getCard(id))
	end
	cards = sgs.QList2Table(cards)
	for _, acard in ipairs(cards) do
		if isCard("Analeptic", acard, player) then return end
	end
	local newcards = {}
	for _, card in ipairs(cards) do
		if not isCard("Analeptic", card, player) and not isCard("Peach", card, player) and not (isCard("ExNihilo", card, player) and player:getPhase() == sgs.Player_Play) then
			table.insert(newcards, card)
		end
	end
	if #newcards < 2 then return end
	sgs.ais[player:objectName()]:sortByKeepValue(newcards)
	
	local card_id1 = newcards[1]:getEffectiveId()
	local card_id2 = newcards[2]:getEffectiveId()
	
	local card_str = ("analeptic:%s[%s:%s]=%d+%d"):format("thzuishang", "to_be_decided", 0, card_id1, card_id2)
	return card_str
end

function sgs.ai_cardsview.thzuishang(self, class_name, player)
	if class_name == "Analeptic" and player:getPhase() ~= sgs.Player_NotActive then
		return cardsView_thzuishang(self, player)
	end
end

function sgs.ai_cardsview.thzuishangv(self, class_name, player)
	if class_name == "Analeptic" then
		local obj_name = player:property("zhouhua_source"):toString()
		local splayer = self.room:findPlayer(obj_name)
		if splayer and splayer:hasSkill("thzuishang") then
			return cardsView_thzhouhua(self, player)
		end
	end
end

sgs.ai_skill_playerchosen.thxugu = sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.ai_skill_use.analeptic = function(self, prompt, method)
	local list = prompt:split(":")
	local from = self.room:findPlayer(list[#list])
	if not from then
		return "."
	end
	if self:getCardsNum("Jink") > 1 then
		return "."
	end
	if self:getCardsNum("Jink") > 0 and not from:canSlash(self.player) then
		return "."
	end
	if getCardsNum("Slash", from, self.player) > 1 and from:canSlash(self.player) then
		local analeptic = self:getCardId("Analeptic")
		if analeptic then
			local card = sgs.Card_Parse(analeptic)
			for _, id in sgs.qlist(card:getSubcards()) do
				if isCard("Jink", id, self.player) then
					return "."
				end
			end
			return analeptic
		end
	end
	return "."
end

sgs.ai_playerchosen_intention.thxugu = 50

--慈航：当你使用的【杀】被目标角色的【闪】抵消时，你可以选择一项，以令此【杀】依然造成伤害：弃置等同于目标角色已损失的体力值数量的牌（不足则全弃）；或令目标角色摸等同于其体力值数量的牌（至多摸五张）。
sgs.thcihang_choice = ""

sgs.ai_skill_invoke.thcihang = function(self, data)
	local effect = data:toSlashEffect()
	local target = effect.to
	if self:isFriend(target) then
		if target:getHp() > 2 and not self:hasHeavySlashDamage(target, effect.slash) then
			sgs.thcihang_choice = "draw"
			return true
		end
		return false
	elseif self:isEnemy(target) then
		if not self:slashIsEffective(effect.slash, target, self.player) then
			return false
		end
		if self:willSkipPlayPhase(target) then
			sgs.thcihang_choice = "draw"
			return true
		end
		local losthp = target:getLostHp()
		if losthp < 2 or self.player:getCardCount() < 2 then
			sgs.thcihang_choice = "discard"
			return true
		end
		local hp = target:getHp()
		if hp < 2 or (hp < 4 and self:hasHeavySlashDamage(target, effect.slash)) then
			sgs.thcihang_choice = "draw"
			return true
		end
	end
	return false
end

sgs.ai_skill_choice.thcihang = function(self, choices, data)
	if sgs.thcihang_choice ~= "" then
		return sgs.thcihang_choice
	end
	local target = data:toPlayer()
	if self:isFriend(target) then
		return "draw"
	end
	return math.random(1, 3) == 1 and "discard" or "draw"
end

sgs.ai_choicemade_filter.skillChoice.thcihang = function(self, player, promptlist)
	local effect = player:getTag("ThCihangData"):toSlashEffect()
	local target = effect.to
	if target then
		if promptlist[#promptlist] == "discard" then
			sgs.updateIntention(player, target, 50)
		end
	end
end

--战操：你的回合外，当你或你的攻击范围内的一名角色成为【杀】的目标时，你可选择一项使该【杀】对其无效：失去1点体力，且该【杀】在结算后置入弃牌堆时，你获得之；或弃置一张非基本牌。
sgs.ai_skill_invoke.thzhancao = function(self, data)
	sgs.thzhancao_throw = nil
	local target = data:toPlayer()
	if self:isFriend(target) then
		local use = self.player:getTag("ThZhancaoData"):toCardUse()
		local slash = use.card
		local need_lost = 0
		if not slash:hasFlag("thzhancao") then
			if slash:isVirtualCard() then
				for _, id in sgs.qlist(slash:getSubcards()) do
					if isCard("Peach", id, self.player) then
						need_lost = 1
						break
					end
				end
			elseif isCard("Peach", slash, self.player) then
				need_lost = 1
			end
			if need_lost > 0 and (self:getHp() > 1 or self:getCardsNum({ "Peach", "Analeptic" }) >= 1) then
				sgs.thzhancao_throw = nil
				return true
			end
		else
			need_lost = -1
		end

		local not_basics = {}
		for _, c in sgs.qlist(self.player:getCards("he")) do
			if c:isKindOf("EquipCard") or c:isKindOf("TrickCard") then
				table.insert(not_basics, c)
			end
		end
		if #not_basics > 0 then
			self:sortByKeepValue(not_basics)
			sgs.thzhancao_throw = not_basics[1]:getEffectiveId()
		end
		if sgs.thzhancao_throw and isCard("Peach", sgs.thzhancao_throw, self.player) then
			sgs.thzhancao_throw = nil
		end
		if self:isWeak(target) and not self:isWeak(self.player) then
			return true
		end
		if sgs.getDefense(target) < sgs.getDefense(self.player) then
			if not slash:hasFlag("thzhancao") and self:getCardsNum("Slash") < 1 and self.player:getHp() > 2 then
				sgs.thzhancao_throw = nil
				return true
			end
			return true
		end
		if self:getOverflow() > 0 and sgs.thzhancao_throw then
			return true
		end
	end
	return false
end

sgs.ai_skill_cardask["@thzhancao"] = function(self, data, pattern)
	if sgs.thzhancao_throw then
		return "$" .. sgs.thzhancao_throw
	else
		return "."
	end
end

sgs.ai_cardneed.thzhancao = function(to, card, self)
	return card:getTypeId() ~= sgs.Card_TypeBasic
end

sgs.ai_choicemade_filter.skillInvoke.thzhancao = function(self, player, promptlist)
	if promptlist[#promptlist] == "yes" then
		local target = self.room:findPlayer(promptlist[#promptlist - 1])
		if target then
			sgs.updateIntention(player, target, -50)
		end
	end
end

sgs.thzhancao_keep_value = {
	Weapon = 6,
	EquipCard = 5,
	TrickCard = 5,
}

--墨迹：当你需要使用或打出一张【杀】或【闪】时，你可以将等同于你体力值数量的牌以任意顺序置于牌堆顶（至多两张），视为你使用或打出一张【杀】或【闪】。
sgs.draw_pile_thmoji = {} --for GlobalRecord

function SmartAI:getMojiCards(player, n, toCard)
	local cards = player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	if #cards < n then
		return {}
	end
	for _, c in ipairs(cards) do
		if isCard(toCard, c, player) then
			return {}
		end
	end
	local ret = {}
	if toCard == "Slash" then
		for i = 1, #cards do
			if not isCard("Slash", cards[i], player) then
				table.insert(ret, cards[i]:getEffectiveId())
			end
			if #ret == n then
				return ret
			end
		end
	elseif toCard == "Jink" then
		for i = 1, #cards do
			if not isCard("Jink", cards[i], player) then
				table.insert(ret, cards[i]:getEffectiveId())
			end
			if #ret == n then
				return ret
			end
		end
	end
	return {}
end

local thmoji_skill = {}
thmoji_skill.name = "thmoji"
table.insert(sgs.ai_skills, thmoji_skill)
thmoji_skill.getTurnUseCard = function(self)
	local n = self.player:getHp()
	if n < 1 then
		return nil
	end
	n = math.min(2, n)
	local cards = self:getMojiCards(self.player, n, "Slash")
	if #cards == n then
		local up, _ = self:askForGuanxing(cards, sgs.Room_GuanxingUpOnly)
		if #up == n then
			up = sgs.reverse(up)
			local card_str = "@ThMojiCard=" .. table.concat(up, "+") .. ":slash"
			return sgs.Card_Parse(card_str)
		end
	end
	return nil
end

sgs.ai_skill_use_func.ThMojiCard = function(card, use, self)
	local slash = sgs.cloneCard("slash")
	for _, id in sgs.qlist(card:getSubcards()) do
		slash:addSubcard(id)
	end
	slash:setSkillName("_thmoji")
	local s_use = { to = sgs.SPlayerList() }
	self:useCardSlash(slash, s_use)
	if s_use.card and not s_use.to:isEmpty() then
		use.card = card
		if use.to then
			use.to = s_use.to
		end
	end
end

sgs.ai_cardsview_valuable.thmoji = function(self, class_name, player)
	if not ("Slash|Jink"):match(class_name) then
		return nil
	end
	local n = player:getHp()
	if n < 1 then
		return nil
	end
	n = math.min(2, n)
	local cards = self:getMojiCards(player, n, class_name)
	if #cards == n then
		local up, _ = self:askForGuanxing(cards, sgs.Room_GuanxingUpOnly)
		if #up == n then
			up = sgs.reverse(up)
			local card_str = "@ThMojiCard=" .. table.concat(up, "+") .. ":" .. string.lower(class_name)
			return card_str
		end
	end
end

sgs.ai_use_priority.ThMojiCard = sgs.ai_use_priority.Slash - 0.01

--缘起：出牌阶段限一次，你可以展示一张牌并亮出牌堆顶的一张牌，若这两张牌颜色相同，你选择一项：将这些牌置入弃牌堆，然后回复1点体力；或将这些牌交给一名角色。
sgs.thyuanqi_target = nil

local thyuanqi_skill = {}
thyuanqi_skill.name = "thyuanqi"
table.insert(sgs.ai_skills, thyuanqi_skill)
thyuanqi_skill.getTurnUseCard = function(self)
	if self.player:isNude() then return end
	if self.player:hasUsed("ThYuanqiCard") then return end

	local function knowFirstCard(player, room)
		return sgs.draw_pile_thmoji[player:objectName()] and table.contains(sgs.draw_pile_thmoji[player:objectName()], room:getDrawPile():first())
	end

	if not self.player:isWounded() then
		if knowFirstCard(self.player, self.room) then
			local first = sgs.Sanguosha(self.room:getDrawPile():first())
			local cards = {}
			for _, c in sgs.qlist(self.player:getCards("he")) do
				if c:sameColorWith(fisrt) then
					table.insert(cards, c)
				end
			end
			if #cards == 0 then
				return
			end
			local card, target = self:getCardNeedPlayer(cards, self.friends_noself, false)
			if card and target then
				sgs.thyuanqi_target = target
				return sgs.Card_Parse("@ThYuanqiCard=" .. card:getEffectiveId())
			else
				self:sortByKeepValue(cards)
				sgs.thyuanqi_target = nil
				return sgs.Card_Parse("@ThYuanqiCard=" .. cards[1]:getEffectiveId())
			end
		else
			local cards = sgs.QList2Table(self.player:getCards("he"))
			self:sortByKeepValue(cards)
			sgs.thyuanqi_target = nil
			return sgs.Card_Parse("@ThYuanqiCard=" .. cards[1]:getEffectiveId())
		end
	else
		if knowFirstCard(self.player, self.room) then
			local first = sgs.Sanguosha(self.room:getDrawPile():first())
			local cards = {}
			for _, c in sgs.qlist(self.player:getCards("he")) do
				if c:sameColorWith(fisrt) then
					table.insert(cards, c)
				end
			end
			if #cards == 0 then
				return
			end
			self:sortByKeepValue(cards)
			sgs.thyuanqi_target = nil
			return sgs.Card_Parse("@ThYuanqiCard=" .. cards[1]:getEffectiveId())
		else
			local cards = sgs.QList2Table(self.player:getCards("he"))
			self:sortByKeepValue(cards)
			sgs.thyuanqi_target = nil
			return sgs.Card_Parse("@ThYuanqiCard=" .. cards[1]:getEffectiveId())
		end
	end
end

sgs.ai_skill_use_func.ThYuanqiCard = function(card, use, self)
	use.card = card
end

sgs.ai_skill_playerchosen.thyuanqi = function(self, targets)
	if sgs.thyuanqi_target and targets:contains(sgs.thyuanqi_target) then return sgs.thyuanqi_target end
	if self.player:isWounded() then return nil end
	local int_list = self.player:getTag("ThYuanqiCards"):toIntList()
	local cards = {}
	for _, id in sgs.qlist(int_list) do
		table.insert(cards, sgs.Sanguosha:getCard(id))
	end
	local _, target = self:getCardNeedPlayer(cards, self.friends_noself, false)
	if target and targets:contains(target) then return target end
	self:sort(self.friends_noself, "handcard")
	for _, p in ipairs(self.friends_noself) do
		if targets:contains(p) then
			return p
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.thyuanqi = -30

--遁甲：出牌阶段限三次，你可以将一张【杀】当【心网密葬】使用；若该角色被此牌所弃置的牌为【闪】，你对其造成1点伤害。
local thdunjia_skill = {}
thdunjia_skill.name = "thdunjia"
table.insert(sgs.ai_skills, thdunjia_skill)
thdunjia_skill.getTurnUseCard = function(self, inclusive)
	local cards = self.player:getCards("he")
	for _, id in sgs.qlist(self.player:getPile("wooden_ox")) do
		cards:prepend(sgs.Sanguosha:getCard(id))
	end
	cards = sgs.QList2Table(cards)

	local slash_card
	self:sortByUseValue(cards, true)

	for _, card in ipairs(cards) do
		if card:isKindOf("Slash") and ((self:getUseValue(card) < sgs.ai_use_value.Dismantlement) or inclusive or self:getOverflow() > 0) then
			local shouldUse = true

			local dummy_use = { isDummy = true }
			if self:getCardsNum("Slash") == 1 then
				self:useBasicCard(card, dummy_use)
				if dummy_use.card then shouldUse = false end
			end

			if shouldUse then
				slash_card = card
				break
			end

		end
	end

	if slash_card then
		local suit = slash_card:getSuitString()
		local number = slash_card:getNumberString()
		local card_id = slash_card:getEffectiveId()
		local card_str = ("dismantlement:thdunjia[%s:%s]=%d"):format(suit, number, card_id)
		local dismantlement = sgs.Card_Parse(card_str)

		assert(dismantlement)

		return dismantlement
	end
end

function sgs.ai_cardneed.thdunjia(to, card, self)
	return card:isKindOf("Slash") and getKnownCard(to, self.player, "Slash", nil, "he") < 3
end

--筹策:你于出牌阶段使用的第一张牌，或点数比你此阶段使用的前一张牌大的牌，可以无视合法性指定一名角色为目标。
--maneuvering-ai.lua SmartAI:searchForAnaleptic
--maneuvering-ai.lua SmartAI:useCardIronChain
--smart-ai.lua SmartAI:sortByDynamicUsePriority
--smart-ai.lua SmartAI:sortByUsePriority
--smart-ai.lua SmartAI:willUsePeachTo
--smart-ai.lua SmartAI:getDistanceLimit
--smart-ai.lua SmartAI:useTrickCard
--smart-ai.lua SmartAI:useEquipCard
--standard_cards-ai.lua SmartAI:useCardSlash
--standard_cards-ai.lua sgs.ai_skill_use.slash
--standard_cards-ai.lua SmartAI:useCardPeach
--standard_cards-ai.lua SmartAI:useCardAmazingGrace
--standard_cards-ai.lua SmartAI:useCardGodSalvation
--standard_cards-ai.lua SmartAI:useCardExNihilo
--standard_cards-ai.lua SmartAI:useCardSnatchOrDismantlement
--standard_cards-ai.lua SmartAI:useCardCollateral
--standard_cards-ai.lua SmartAI:useCardLightning
--standard_cards-ai.lua SmartAI:useCardPurpleSong
--standard_cards-ai.lua SmartAI:useCardKnownBoth
--standard_cards-ai.lua SmartAI:useCardBurningCamps
local thchouce_skill = {}
thchouce_skill.name = "thchouce"
table.insert(sgs.ai_skills, thchouce_skill)
thchouce_skill.getTurnUseCard = function(self, inclusive)
	if self.player:hasFlag("Global_ThChouceFailed") or self.player:getMark("ThChouce") > 12 or self.player:hasFlag("ThChouceUse") then return end
	return sgs.Card_Parse("@ThChouceCard=.")
end

sgs.ai_skill_use_func.ThChouceCard = function(card, use, self)
	use.card = card
end

sgs.ai_skill_use.peach = function(self, prompt)
	local use = { isDummy = true, to = sgs.SPlayerList() }
	local peach_str = self:getCardId("Peach")
	local peach
	if peach_str then
		peach = sgs.Card_Parse(peach_str)
	end
	if peach then
		self:useCardPeach(peach, use)
		if use.card and not use.to:isEmpty() then
			return peach:toString() .. "->" .. use.to:first():objectName()
		end
	end
	return "."
end

sgs.ai_skill_use["peach+analeptic"] = function(self, prompt)
	local use = { isDummy = true, to = sgs.SPlayerList() }
	local peach_str = self:getCardId("Peach")
	local peach
	if peach_str then
		peach = sgs.Card_Parse(peach_str)
	end
	local analeptic_str = self:getCardId("Analeptic")
	local analeptic
	if analeptic_str then
		analeptic = sgs.Card_Parse(analeptic_str)
	end
	if peach and analeptic and peach:getNumber() < analeptic:getNumber() then
		self:useCardPeach(peach, use)
		if use.card and not use.to:isEmpty() then
			return peach:toString() .. "->" .. use.to:first():objectName()
		end
	elseif peach and analeptic then
		self:useCardPeach(analeptic, use)
		if use.card and not use.to:isEmpty() then
			return analeptic:toString() .. "->" .. use.to:first():objectName()
		end
	elseif peach then
		self:useCardPeach(peach, use)
		if use.card and not use.to:isEmpty() then
			return peach:toString() .. "->" .. use.to:first():objectName()
		end
	elseif analeptic then
		self:useCardPeach(peach, use)
		if use.card and not use.to:isEmpty() then
			return peach:toString() .. "->" .. use.to:first():objectName()
		end
	end
	return "."
end

sgs.ai_use_priority.ThChouceCard = 10000

--占筮：锁定技，出牌阶段结束时，若你于此阶段使用的牌不少于三张，且全部发动了“筹策”，你在此回合结束后进行一个额外的回合，且于额外回合的回合开始时获得技能“幻葬”直到回合结束。
--无

--幻葬：锁定技，出牌阶段结束时，你进行一次判定，若结果为黑色，你失去1点体力。
--无

--紫云：锁定技，你不能成为【玄海仙鸣】或【枯羽华庭】的目标。
--无

--垂迹：弃牌阶段结束时，若你于此阶段弃置了两张或更多的手牌，或每当你于回合外失去牌时，你可进行一次判定，若为红色，令一名角色回复1点体力；若为黑色，弃置一名其他角色的一张牌。
sgs.ai_skill_invoke.thchuiji = function(self, data)
	for _, p in ipairs(self.friends) do
		if p:isWounded() then
			return true
		end
	end
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if p:isWounded() then
			return false
		end
	end
	return true
end

sgs.ai_skill_playerchosen.thchuiji = function(self, targets)
	local str = self.player:getTag("ThChuijiTarget"):toString()
	if str == "recover" then
		targets = sgs.QList2Table(targets)
		self:sort(targets, "defense")
		for _, p in ipairs(targets) do
			if self:isFriend(p) then
				return p
			end
		end
		return targets[#targets]
	elseif str == "discard" then
		return self:findPlayerToDiscard("he", false, true, targets)
	end
end

sgs.ai_playerchosen_intention.thchuiji = function(self, from, to)
	local str = from:getTag("ThChuijiTarget"):toString()
	if str == "recover" then
		sgs.updateIntention(from, to, -50)
	end
end

sgs.ai_choicemade_filter.cardChosen.thchuiji = sgs.ai_choicemade_filter.cardChosen.snatch

--灵压：每当其他角色于你的回合使用红色牌，在结算后你可以令其选择一项：令你摸一张牌；或令你弃置其一张牌。
sgs.ai_skill_invoke.thlingya = true

sgs.ai_skill_choice.thlingya = function(self, choices, data)	
	local yukari = data:toPlayer()
	if yukari and string.find(choices, "discard") then
		if self:isEnemy(yukari) then
			local LetDiscard = false
			if not yukari:canDiscard(self.player, "h") and self.player:hasSkills(sgs.lose_equip_skill) then
				LetDiscard  = true
			end
			if self:needKongcheng(p) and self:getHandcardNum() == 1 and not yukari:canDiscard(self.player, "e") then
				LetDiscard  = true
			end
			if self.player:hasSkills(sgs.lose_equip_skill) and self:needKongcheng(p) and self:getHandcardNum() == 1 then
				LetDiscard  = true
			end
			if LetDiscard then return "discard" end
		elseif self.player:hasSkills(sgs.lose_equip_skill) and self.player:canDiscard(self.player, "e") then
			return "discard"
		end
	end
	return "letdraw"
end

--黑幕：出牌阶段限一次，当你使用一张牌指定目标时，你可以令一名其他角色成为此牌的使用者。
sgs.ai_skill_playerchosen.thheimu = function(self, targets)
	local cardUse = self.player:getTag("ThHeimuCardUse"):toCardUse()
	local card = cardUse.card
	local isRed = card:isRed()

	--case1 灵压敌人
	local goodLingyaCard = "god_salvation|amazing_grace|iron_chain|peach|analeptic"
	local isGoodLingyaCard = card:isKindOf("EquipCard") or card:isKindOf("DelayedTrick") or goodLingyaCard:match(card:objectName())
	if self.player:hasSkill("thlingya") and isGoodLingyaCard and isRed then
		if #self.enemies > 0 then
			self:sort(self.enemies, "defense")
			return self.enemies[1]
		end
	end

	--case2 助队友收反或使主公杀忠掉牌
	local isDamageCard = not card:isKindOf("FireAttack") and sgs.dynamic_value.damage_card[card:getClassName()]
	if isDamageCard then
		local lord = self.room:getLord()
		if self:isFriend(lord) then
			local weakRebel
			for _, p in sgs.qlist(cardUse.to) do
				if p:getHp() <= 1 and self:isEnemy(p) then
					weakRebel = p
					break
				end
			end
			if weakRebel then
				for _, p in sgs.qlist(targets) do
					if self:isFriend(p) and p:hasSkills(sgs.cardneed_skill) then
						if (card:isKindOf("TrickCard") and self:hasTrickEffective(card, weakRebel, p))
								or (card:isKindOf("Slash") and self:slashIsEffective(card, weakRebel, p)) then
							return p
						end
					end
				end
			end
		else
			local weakLoyalist
			for _,p in sgs.qlist(cardUse.to) do
				if p:getHp() <= 1 and self:isEnemy(p) then
					weakLoyalist = p
					break
				end
			end
			if weakLoyalist then
				if targets:contains(lord) then
					if (card:isKindOf("TrickCard") and self:hasTrickEffective(card, weakLoyalist, lord))
							or (card:isKindOf("Slash") and self:slashIsEffective(card, weakLoyalist, lord)) then
						return lord
					end
				end
			end
		end
	end

	--case3  一般灵压 针对队友
	if isRed and self.player:hasSkill("thlingya") then
		for _, p in sgs.qlist(targets) do
			if self:isFriend(p) then
				return p
			end
		end
	end
	return nil
end

--寒魄：锁定技，防止你对其他角色造成的火焰伤害，防止你受到的火焰伤害。
--smart-ai.lua SmartAI:damageIsEffective

--争冠：每当一名其他角色跳过摸牌阶段后，你可以摸两张牌；每当一名其他角色跳过出牌阶段后，你可以无视距离对一名其他角色使用一张【杀】。
sgs.ai_skill_invoke.thzhengguan = true

--氷瀑：限定技，出牌阶段，你可以令所有其他有牌的角色做出选择：打出一张【闪】，或令你依次弃置其两张牌。
local thbingpu_skill = {}
thbingpu_skill.name = "thbingpu"
table.insert(sgs.ai_skills, thbingpu_skill)
thbingpu_skill.getTurnUseCard = function(self)
	if self.player:getMark("@bingpu") <= 0 then return end
	local good, bad = 0, 0
	local lord = self.room:getLord()
	if self.role ~= "rebel" and lord and self:isWeak(lord) then return end
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:isNude() then
			continue
		end
		if self:isWeak(player) then
			if self:isFriend(player) then bad = bad + 1
			else good = good + 1
			end
		end
	end
	if good == 0 then return end

	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:isNude() then
			continue
		end
		local hp = math.max(player:getHp(), 1)
		if getCardsNum("Analeptic", player, self.player) > 0 then
			if self:isFriend(player) then good = good + 1.0 / hp
			else bad = bad + 1.0 / hp
			end
		end

		local has_jink = (getCardsNum("Jink", player, self.player) > 0)
		if not has_jink then
			if self:isFriend(player) then good = good + 1
			else bad = bad + 1
			end
		end

		if getCardsNum("Jink", player) == 0 then
			local lost_value = player:getCardCount()
			local hp = math.max(player:getHp(), 1)
			if self:isFriend(player) then bad = bad + lost_value / hp
			else good = good + lost_value / hp
			end
		end
	end

	if good > bad + 1 then return sgs.Card_Parse("@ThBingpuCard=.") end
end

sgs.ai_skill_use_func.ThBingpuCard = function(card, use, self)
	use.card = card
end

sgs.ai_use_priority.ThBingpuCard = sgs.ai_use_priority.Slash + 0.09

--冬末：结束阶段开始时，你可以指定一至X名其他角色（X为你已损失的体力值），你和这些角色将各自的人物牌翻面，并各摸一张牌。
sgs.ai_skill_use["@@thdongmo"] = function(self, prompt)
	local targets = {}
	local players = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	self:sort(players, "threat")
	for _, p in ipairs(players) do
		if (self:isFriend(p) and p:faceUp() == p:hasSkill("thshiting"))
				or (self:isEnemy(p) and p:faceUp() ~= p:hasSkill("thshiting")) then
			table.insert(targets, p:objectName())
		end
		if #targets >= self.player:getLostHp() then
			break
		end
	end
	if #targets == 0 then
		return "."
	elseif self.player:getLostHp() - #targets > 1 then
		local can = false
		if not self.player:faceUp() then
			can = true
		end
		if can then
			return "@ThDongmoCard=.->" .. table.concat(targets, "+")
		end
	else
		return "@ThDongmoCard=.->" .. table.concat(targets, "+")
	end
	return "."
end

sgs.ai_card_intention.ThDongmoCard = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if to:faceUp() ~= to:hasSkill("thshiting") then
			sgs.updateIntention(from, to, 60)
		else
			sgs.updateIntention(from, to, -80)
		end
	end
end

--凛寒：当你使用或者打出一张【闪】时，你可以摸一张牌。
sgs.ai_skill_invoke.thlinhan = true

--复声：锁定技，其他角色每令你回复1点体力，该角色摸一张牌；每当你受到一次伤害后，伤害来源须交给你一张红桃手牌，否则失去1点体力。
sgs.ai_skill_cardask["@thfusheng-heart"] = function(self, data)
	if self:needToLoseHp() then return "." end
	local damage = data:toDamage()
	if self:isFriend(damage.to) then return end

	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Heart
			and not (isCard("Peach", card, self.player) or (isCard("ExNihilo", card, self.player) and self.player:getPhase() == sgs.Player_Play)) then
			return card:getEffectiveId()
		end
	end
	return "."
end

function sgs.ai_slash_prohibit.thfusheng(self, from, to, card)
	if from:hasSkill("ikxuwu") or (from:hasSkill("ikwanhun") and from:distanceTo(to) > 0 and from:distanceTo(to) < 2) then return false end
	if from:hasFlag("IkJieyouUsed") then return false end
	if self:needToLoseHp(from) then return false end
	if from:getHp() > 3 then return false end

	local n = 0
	local cards = from:getHandcards()
	for _, hcard in sgs.qlist(cards) do
		if hcard:getSuit() == sgs.Card_Heart and not (isCard("Peach", hcard, to) or isCard("ExNihilo", hcard, to)) then
			if not hcard:isKindOf("Slash") then return false end
			n = n + 1
			if n > 1 then return false end
		end
	end
	if n == 1 then return card:getSuit() == sgs.Card_Heart end
	return self:isWeak(from)
end

sgs.ai_need_damaged.thfusheng = function(self, attacker, player)
	if attacker and self:isEnemy(attacker, player) and self:isWeak(attacker) then
		return true
	end
	return false
end

--幻法：出牌阶段限一次，你可以将一张红桃手牌交给一名其他角色，然后你获得该角色的一张牌并将该牌交给除该角色外的一名角色。
thhuanfa_skill = {}
thhuanfa_skill.name = "thhuanfa"
table.insert(sgs.ai_skills, thhuanfa_skill)
thhuanfa_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("ThHuanfaCard") then
		return sgs.Card_Parse("@ThHuanfaCard=.")
	end
end

sgs.ai_skill_use_func.ThHuanfaCard = function(card, use, self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)

	local target
	for _, friend in ipairs(self.friends_noself) do
		if friend:hasSkills(sgs.lose_equip_skill) and not friend:getEquips():isEmpty() and not friend:hasSkill("manjuan") then
			target = friend
			break
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if self:getDangerousCard(enemy) then
				target = enemy
				break
			end
		end
	end
	if not target then
		for _, friend in ipairs(self.friends_noself) do
			if self:needToThrowArmor(friend) and not friend:hasSkill("manjuan") then
				target = friend
				break
			end
		end
	end
	if not target then
		self:sort(self.enemies, "handcard")
		for _, enemy in ipairs(self.enemies) do
			if self:getValuableCard(enemy) then
				target = enemy
				break
			end
			if target then break end

			local cards = sgs.QList2Table(enemy:getHandcards())
			local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), enemy:objectName())
			if not enemy:isKongcheng() and not enemy:hasSkills("ikyindie+ikguiyue") then
				for _, cc in ipairs(cards) do
					if (cc:hasFlag("visible") or cc:hasFlag(flag)) and (cc:isKindOf("Peach") or cc:isKindOf("Analeptic")) then
						target = enemy
						break
					end
				end
			end
			if target then break end

			if self:getValuableCard(enemy) then
				target = enemy
				break
			end
			if target then break end
		end
	end
	if not target then
		for _, friend in ipairs(self.friends_noself) do
			if friend:hasSkills("ikyindie+ikguiyue") and not friend:hasSkill("manjuan") then
				target = friend
				break
			end
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isNude() and enemy:hasSkill("manjuan") then
				target = enemy
				break
			end
		end
	end

	if target then
		local heart_card
		if self:isFriend(target) then
			for _, card in ipairs(cards) do
				if card:getSuit() == sgs.Card_Heart then
					heart_card = card
					break
				end
			end
		else
			for _, card in ipairs(cards) do
				if card:getSuit() == sgs.Card_Heart and not isCard("Peach", card, target) and not isCard("ExNihilo", card, target) then
					heart_card = card
					break
				end
			end
		end

		if heart_card then
			use.card = sgs.Card_Parse("@ThHuanfaCard=" .. heart_card:getEffectiveId())
			if use.to then use.to:append(target) end
		end
	end
end

sgs.ai_skill_playerchosen.thhuanfa = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if (player:getHandcardNum() <= 2 or player:getHp() < 2) and self:isFriend(player) and not self:needKongcheng(player, true) and not player:hasSkill("manjuan") then
			return player
		end
	end
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player) and not self:needKongcheng(player, true) and not player:hasSkill("manjuan") then
			return player
		end
	end
	return self.player
end

sgs.ai_card_intention.ThHuanfaCard = function(self, card, from, tos)
	local rcard = sgs.Sanguosha:getCard(card:getEffectiveId())
	if self:isValuableCard(rcard) then return end
	local to = tos[1]
	if not to:hasSkill("manjuan") and (self:needToThrowArmor(to) or (to:hasSkills(sgs.lose_equip_skill) and not to:getEquips():isEmpty()) or to:hasSkill("ikyindie")) then
	else
		sgs.updateIntention(from, to, 40)
	end
end

sgs.thhuanfa_suit_value = {
	heart = 3.9
}

sgs.ai_cardneed.thhuanfa = function(to, card)
	return card:getSuit() == sgs.Card_Heart
end

--骚葬：弃牌阶段结束时，你每于此阶段弃置了一种类别的牌，你可以依次弃置一名其他角色的一张手牌。
sgs.ai_skill_playerchosen.thsaozang = function(self, targets)
	return self:findPlayerToDiscard("h", false, true, targets)
end

sgs.ai_choicemade_filter.cardChosen.thsaozang = sgs.ai_choicemade_filter.cardChosen.snatch

--絮曲：你的回合外，当你失去手牌时，你可以令一名其他角色摸一张牌。
sgs.ai_skill_playerchosen.thxuqu = function(self, targets)
	return self:findPlayerToDraw(false, 1)
end

sgs.ai_playerchosen_intention.thxuqu = -20

--【苦戒】ai
local thkujiev_skill = {}
thkujiev_skill.name = "thkujiev"
table.insert(sgs.ai_skills, thkujiev_skill)
thkujiev_skill.getTurnUseCard = function(self)
	if self.player:hasFlag("ForbidThKujie") then return nil end
	local reds={}
	for _,c in sgs.qlist(self.player:getCards("he")) do
		if c:isRed() and c:isKindOf("BasicCard") then
			table.insert(reds,c)
		end
	end
	if #reds==0 then return nil end
	self:sortByKeepValue(reds)
	return sgs.Card_Parse("@ThKujieCard=" .. reds[1]:getEffectiveId())
end
sgs.ai_skill_use_func.ThKujieCard = function(card, use, self)
	local targets ={}
	for _,p in sgs.qlist(self.room:findPlayersBySkillName("thkujie")) do
		if  self.player:inMyAttackRange(p) and not p:hasFlag("ThKujieInvoked") then
			table.insert(targets,p)
		end
	end
	if #targets==0 then return nil end
	self:sort(targets, "hp")
	local good_target 
	for _,p in pairs (targets) do
		if self:isEnemy(p) then
			if p:getHp()==1 and self:getAllPeachNum(p)<=0 then
				good_target = p
				break
			end
		elseif self:isFriend(p) then
			if p:isWounded() and p:getHp()>1 then
				good_target = p
				break
			end
		end
	end
	if good_target then
		use.card = card
		if use.to then
			use.to:append(good_target)
			return
		end
	end
end
sgs.ai_card_intention.ThKujieCard = function(self, card, from, tos)
	for _, to in pairs(tos) do
		if to:getHp()<=1 then
			sgs.updateIntention(from, to, 80)
		else
			sgs.updateIntention(from, to, -20)
		end
	end
end

--【廕庇】ai
sgs.ai_skill_invoke.thyinbi = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.to) then
		if damage.to:getLostHp() >= damage.damage and  self.player:getHp() > damage.damage then
			local isSlash 
			if damage.card and damage.card:isKindOf("Slash") then
				isSlash= true
			end
			return not self:needToLoseHp(damage.to, damage.from, isSlash, true)
		end
	end
	return false
end
sgs.ai_choicemade_filter.skillInvoke.thyinbi = function(self, player, promptlist)
	local to=player:getTag("thyinbiDamage"):toDamage().to
	if to and promptlist[#promptlist] == "yes" then
		sgs.updateIntention(player, to, -80)
	end
end




--【灵蝶】ai
local function countKnownCards(target)
		local count=0
		for _, card in sgs.qlist(target:getHandcards()) do
			--flag的情况其实可以不要。。。
			local flag = string.format("%s_%s_%s", "visible", global_room:getCurrent():objectName(), target:objectName())
			if  card:hasFlag("visible") or card:hasFlag(flag) then	
				count=count+1
			end
		end
		return count
	end
local lingdieCompare_func = function(a, b)
	return	countKnownCards(a)>countKnownCards(b)
end
	
local thlingdie_skill = {}
thlingdie_skill.name = "thlingdie"
table.insert(sgs.ai_skills, thlingdie_skill)
thlingdie_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ThLingdieCard") then return nil end
	if #self.friends_noself ==0 then return nil end
	local cards =sgs.QList2Table(self.player:getCards("he"))
	if #cards==0 then return nil end
	
	--没补牌技能时，防御太虚
	if not self.player:hasSkill("thwushou") and #cards <3 then
	--sgs.getDefense(self.player, gameProcess)
		return nil 
	end

	self:sortByKeepValue(cards)
	return sgs.Card_Parse("@ThLingdieCard=" .. cards[1]:getEffectiveId())
	--目前敌友都可以属于看牌目标，也就不预先检测目标了
end
sgs.ai_skill_use_func.ThLingdieCard = function(card, use, self)

	local good_enemy
	if #self.enemies>0 then
		table.sort(self.enemies, lingdieCompare_func)
		good_enemy=self.enemies[1]
	end
	local good_friend
	if good_enemy then
		for _,p in pairs (self.friends_noself) do
			--考虑急火？ canslash？
			if p:inMyAttackRange(good_enemy)  then
				good_friend =p
				break
			end
		
		end
	end
	use.card = card
	if use.to then
		if good_friend then
			use.to:append(good_friend)
		else
			use.to:append(self.friends_noself[math.random(1,#self.friends_noself)])
		end
		return
	end
end
sgs.ai_card_intention.ThLingdieCard = -50
sgs.ai_skill_playerchosen.thlingdie = function(self, targets)
	if #self.enemies>0 then
		table.sort(self.enemies, lingdieCompare_func)
		good_enemy=self.enemies[1]
	end
	if good_enemy and not good_enemy:isKongcheng() then
		return good_enemy
	end
	return targets:first()
end
sgs.ai_playerchosen_intention.thlingdie =function(self, from, to)
	if not self:isFriend(from,to) then
		sgs.updateIntention(from, to, 20)
	end
end
--灵蝶优先度应该很低。。。

--【无寿】ai
sgs.ai_need_damaged.thwushou = function(self, attacker, player)
	--卖血条件：体力值大于1，且能补3张以上
	if attacker and attacker:hasSkill("ikxuwu") then return false end
	local num = 4 - player:getHp()
	return num >= 2
end

--【浮月】ai
local thfuyuev_skill = {}
thfuyuev_skill.name = "thfuyuev"
table.insert(sgs.ai_skills, thfuyuev_skill)
thfuyuev_skill.getTurnUseCard = function(self)
	if self.player:getKingdom()~="yuki" then return nil end
	if self.player:isKongcheng() or self:needBear()  or self.player:hasFlag("ForbidThFuyue") then return nil end
	return sgs.Card_Parse("@ThFuyueCard=.")
end
sgs.ai_skill_use_func.ThFuyueCard = function(card, use, self)
	local lord
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasLordSkill("thfuyue") and not player:isKongcheng() 
		and not player:hasFlag("ThFuyueInvoked") and player:isWounded()
		then
			lord=player
			break
		end
	end
	if not lord then return nil end
	
	--暂时不考虑反贼获利
	if self:isEnemy(lord)  then
		return nil
	end
	
	
	
	
	local cards = self.player:getHandcards()
	local max_num = 0, max_card
	local min_num = 14, min_card
		for _, hcard in sgs.qlist(cards) do
			if hcard:isKindOf("Peach") then continue end
			if hcard:getNumber() > max_num then
				max_num = hcard:getNumber()
				max_card = hcard
			end

			if hcard:getNumber() <= min_num then
				if hcard:getNumber() == min_num then
					if min_card and self:getKeepValue(hcard) > self:getKeepValue(min_card) then
						min_num = hcard:getNumber()
						min_card = hcard
					end
				else
					min_num = hcard:getNumber()
					min_card = hcard
				end
			end
		end
	if not min_card then return nil end
	
	--很大概率主公赢不了
	if min_card:getNumber()>=12 then return nil end
	if min_card:getNumber()>9 and lord:getHandcardNum()<=4 then
		local lord_card = self:getMaxCard(lord)
		if not lord_card or lord_card:getNumber() < min_card:getNumber() then
			return nil
		end
	end
	if self:isFriend(lord) then
		self.thfuyue_card = min_card:getEffectiveId()
		use.card = card
		if use.to then use.to:append(lord) end
		return
	end
end
--响应拼点者的应对
function sgs.ai_skill_pindian.thfuyue(minusecard, self, requestor, maxcard)
	return self:getMaxCard()
end

sgs.ai_choicemade_filter.pindian.thfuyue = function(self, from, promptlist)
	local number = sgs.Sanguosha:getCard(tonumber(promptlist[4])):getNumber()
	local lord = findPlayerByObjectName(self.room, promptlist[5])
	if not lord then return end
	
	if number < 6 then sgs.updateIntention(from, lord, -60)
	elseif number > 8 and lord:getHandcardNum()<=4 and self:isEnemy(lord,from) then 
	--反贼拼点？
		sgs.updateIntention(from, lord, 60) 
	end
end
