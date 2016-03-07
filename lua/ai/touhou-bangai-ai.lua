--辨方：每当你使用【杀】对一名其他角色造成一次伤害后，或一名其他角色使用一张【杀】对你造成一次伤害后，你可以进行X次判定，每有一张判定牌为红色，你便可以获得该角色的一张牌（X为你已损失的体力值，且至少为1）。
sgs.ai_skill_invoke.thbianfang = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if target:objectName() == self.player:objectName() then
		target = damage.from
	end
	if target and not target:isNude() and target:isAlive() and self:isEnemy(target) and not self:doNotDiscard(target) then
		return true
	end
	if target and self:needToThrowArmor(target) and self:isFriend(target) then
		return true
	end
	return false
end

--授卷：出牌阶段开始时，你可以摸两张牌，若如此做，此阶段内每当一名其他角色失去牌时，你须交给其一张牌。
sgs.ai_skill_invoke.thshoujuan = function(self, data)
	local use = { isDummy = true }
	self:activate(use)
	if not use.card then
		return true
	end
end

sgs.ai_skill_cardask["@thshoujuan"] = function(self, data, pattern, target)
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByUseValue(cards, false)
	if self:isFriend(target) then
		local card, _ = self:getCardNeedPlayer(cards, { target }, false)
		if card then
			return "$" .. card:getEffectiveId()
		end
	else
		for _, c in ipairs(cards) do
			if isCard("Peach", c, target) then
				continue
			else
				return "$" .. c:getEffectiveId()
			end
		end
	end
	return "$" .. c:getEffectiveId()
end

--谧契：结束阶段开始时，若你的手牌数为全场最少的（或之一），你可以弃置场上的至多等同于你手牌数的牌。
sgs.ai_skill_use["@@thmiqi"] = function(self, prompt, method)
	local n = self.player:getHandcardNum()
	local targets = self:findPlayerToDiscard("ej", true, true, nil, true)
	local tos = {}
	for _, t in ipairs(targets) do
		table.insert(tos, t:objectName())
		if #tos == n then
			break
		end
	end
	if #tos > 0 then
		return "@ThMiqiCard=.->" .. table.concat(tos, "+")
	end
	return "."
end

sgs.ai_choicemade_filter.cardChosen.thmiqi = sgs.ai_choicemade_filter.cardChosen.snatch

--疾步：锁定技，当你计算与其他角色的距离时，始终-1。
--无

--织月：当你使用【杀】时，可以进行一次判定，若为黑色，额外指定一个目标；若为红色，此【杀】指定目标后，弃置目标角色一张牌。
sgs.ai_skill_invoke.thzhiyue = function(self, data)
	local use = data:toCardUse()
	for _, p in sgs.qlist(use.to) do
		if self:isFriend(p) and self.player:canDiscard(p, "he") and not self:needToThrowArmor(p) then
			return false
		end
	end
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:isEnemy(p) and self.player:canSlash(p, use.card) and not use.to:contains(p) then
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.thzhiyue = function(self, targets)
	local slash = self.player:getTag("ThZhiyueSlash"):toCard()
	targets = sgs.QList2Table(targets)
	self:sort(targets, "defenseSlash")
	for _, target in ipairs(targets) do
		if self:isEnemy(target) and not self:slashProhibit(slash, target) and sgs.isGoodTarget(target, targets, self) and self:slashIsEffective(slash, target) then
			return target
		end
	end
	for _, target in ipairs(targets) do
		if self:isEnemy(target) then
			return target
		end
	end
	for _, target in ipairs(targets) do
		if self:isFriend(target) and not self:slashIsEffective(slash, target) then
			return target
		end
	end
	return targets[#targets]
end

sgs.ai_playerchosen_intention.thzhiyue = function(self, from, to)
	local tos = sgs.SPlayerList()
	tos:append(to)
	local slash = from:getTag("ThZhiyueSlash"):toCard()
	sgs.ai_card_intention.Slash(self, slash, from, tos)
end

--忠节：每当你受到一次伤害后，你可以展示一名角色的所有手牌，每少一种类型的牌，其摸一张牌。
sgs.ai_skill_playerchosen.thzhongjie = function(self, targets)
	local getKnownTypeNum = function(target)
		local types = {}
		local unknown = 0
		local flag = ("%s_%s_%s"):format("visible", self.player:objectName(), target:objectName())
		for _, c in sgs.qlist(target:getHandcards()) do
			if self.player:objectName() == target:objectName() or c:hasFlag("visible") or c:hasFlag(flag) then
				if not table.contains(types, c:getType()) then
					table.insert(types, c:getType())
				end
				continue
			else
				unknown = unknown + 1
			end
		end
		return #types, unknown
	end
	local thzhongjie_comp = function(a, b)
		local a1, a2 = getKnownTypeNum(a)
		local b1, b2 = getKnownTypeNum(b)
		if a1 ~= b1 then
			return a2 < b2
		else
			return a1 < b1
		end
	end
	local friends = {}
	for _, p in sgs.qlist(targets) do
		if self:isFriend(p) then
			local n = getKnownTypeNum(p)
			if n ~= 3 then
				table.insert(friends, p)
			end
		end
	end
	if #friends == 0 then return nil end
	table.sort(friends, thzhongjie_comp)
	return friends[1]
end

sgs.ai_playerchosen_intention.thzhongjie = -20

--虚魅：出牌阶段限一次，你可以弃置一张基本牌并亮出牌堆顶的三张牌，然后令一名角色获得其中一种类别的牌，将其余的牌置入弃牌堆。若如此做，该角色不能使用或打出该类别的牌，直到回合结束。
local thxumei_skill = {}
thxumei_skill.name = "thxumei"
table.insert(sgs.ai_skills, thxumei_skill)
thxumei_skill.getTurnUseCard = function(self)
	if self.player:canDiscard(self.player, "h") and not self.player:hasUsed("ThXumeiCard") then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByUseValue(cards, true)
		for _, c in ipairs(cards) do
			if c:isKindOf("BasicCard") and not self:isValuableCard(c) then
				return sgs.Card_Parse("@ThXumeiCard=" .. c:getEffectiveId())
			end
		end
	end
end

sgs.ai_skill_use_func.ThXumeiCard = function(card, use, self)
	use.card = card
end

sgs.ai_skill_choice.thxumei = function(self, choices, data)
	sgs.thxumei_target = nil
	local ids = data:toIntList()
	local choice_map = {}
	for _, id in sgs.qlist(ids) do
		local c = sgs.Sanguosha:getCard(id)
		local str = c:getType()
		if str == "skill" then continue end
		if not choice_map[str] then choice_map[str] = {} end
		table.insert(choice_map[str], c)
	end
	for _, id in sgs.qlist(ids) do
		local c = sgs.Sanguosha:getCard(id)
		self:sort(self.friends_noself, "defense")
		for _, p in ipairs(self.friends_noself) do
			if isCard("Peach", c, p) then
				sgs.thxumei_target = p
				return c:getType()
			end
		end
	end
	if #choice_map < 3 then
		for str, t in pairs(choice_map) do
			if #t > 1 then
				return str
			end
		end
	end
	local target, cardId = sgs.ai_skill_askforyiji.nosyiji(self, sgs.QList2Table(ids))
	sgs.thxumei_target = target
	return sgs.Sanguosha:getCard(cardId):getType()
end

sgs.ai_skill_playerchosen.thxumei = function(self, targets)
	if sgs.thxumei_target then
		return sgs.thxumei_target
	end
	local dummy = self.player:getTag("ThXumeiDummy"):toCard()
	local target, _ = sgs.ai_skill_askforyiji.nosyiji(self, sgs.QList2Table(dummy:getSubcards()))
	return target or self.player
end

sgs.ai_use_priority.ThXumeiCard = -2

--隙境：你的回合外，每当你的非装备牌进入弃牌堆后，你可以用一张相同颜色的手牌替换之。
sgs.ai_skill_cardask["@thxijing"] = function(self, data)
	local card = data:toCard()
	if card then
		local h_cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(h_cards)
		for _, c in ipairs(h_cards) do
			if c:sameColorWith(card) then
				if self:getKeepValue(card, false) > self:getKeepValue(c) then
					return "$" .. c:getEffectiveId()
				end
			end
		end
	else
		self.room:writeToConsole("no Card!")
	end
	return "."
end

--梦违：结束阶段开始时，若你的手牌小于两张，你可以将手牌补至两张；其他角色的准备阶段开始时，若你没有手牌，你可以摸一张牌。
--无

--死镰：锁定技，专属技，你的武器牌均视为【杀】；你获得即将进入你装备区的武器牌；若你的装备区没有武器牌，你视为装备着【离魂之镰】。
--无

--灵战：每当你使用【杀】对目标角色造成一次伤害后，你可以进行一次判定，将非红桃的判定牌置于你的人物牌上称为“幻”，你可以将一张“幻”当【杀】使用或者打出。
sgs.ai_skill_invoke.thlingzhan = true

local thlingzhan_skill = {}
thlingzhan_skill.name = "thlingzhan"
table.insert(sgs.ai_skills, thlingzhan_skill)
thlingzhan_skill.getTurnUseCard = function(self)
	if self.player:getPile("lingzhanpile"):isEmpty() then
		return
	end
	for i = 0, self.player:getPile("lingzhanpile"):length() - 1 do
		local slash = sgs.Sanguosha:getCard(self.player:getPile("lingzhanpile"):at(i))
		local slash_str = ("slash:thlingzhan[%s:%s]=%d"):format(slash:getSuitString(), slash:getNumberString(), self.player:getPile("lingzhanpile"):at(i))
		local lingzhanslash = sgs.Card_Parse(slash_str)
		if self:slashIsAvailable(self.player, lingzhanslash) then
			return lingzhanslash
		end
	end
end

sgs.ai_view_as.thlingzhan = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceSpecial and player:getPileName(card_id) == "lingzhanpile" then
		return ("slash:thlingzhan[%s:%s]=%d"):format(suit, number, card_id)
	end
end

--衍梦：锁定技，其他角色不能令你的人物技能无效或失去。
--无

--琴韶：弃牌阶段开始时，若你的手牌数大于体力值，你可以令一名其他角色摸X张牌；若你的手牌数小于体力值，你可以摸X张牌（X为你的手牌数与体力值之差）。
sgs.ai_skill_playerchosen.thqinshao = function(self, targets)
	local n = self.player:getHandcardNum() - self.player:getHp()
	return self:findPlayerToDraw(false, n)
end
	
sgs.ai_skill_invoke.thqinshao = true

sgs.ai_skill_playerchosen.thqinshao = -40

--星屑：出牌阶段限一次，你可以弃置一张手牌，然后将一名角色装备区内的全部牌置于你的人物牌上，此回合结束时，令其依次获得并使用这些牌。
local thxingxie_skill = {}
thxingxie_skill.name = "thxingxie"
table.insert(sgs.ai_skills, thxingxie_skill)
thxingxie_skill.getTurnUseCard = function(self)
	if self.player:canDiscard(self.player, "h") and not self.player:hasUsed("ThXingxieCard") then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByUseValue(cards, true)
		for _, c in ipairs(cards) do
			if not self:isValuableCard(c) then
				return sgs.Card_Parse("@ThXingxieCard=" .. c:getEffectiveId())
			end
		end
	end
end

sgs.ai_skill_use_func.ThXingxieCard = function(card, use, self)
	local slash = self:getCard("Slash")
	if slash and self:slashIsAvailable(self.player, slash) then
		local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
		self:useCardSlash(slash, dummy_use)
		if dummy_use.card then
			for _, p in sgs.qlist(dummy_use.to) do
				if self:isEnemy(p) and p:getArmor() and not self:needToThrowArmor(p) then
					use.card = card
					if use.to then
						use.to:append(p)
					end
					return
				end
			end
		end
	end
	for _, p in ipairs(self.friends) do
		if p:hasArmorEffect("silver_lion") and p:getArmor() and p:isWounded() then
			use.card = card
			if use.to then
				use.to:append(p)
			end
			return
		end
	end
	for _, p in ipairs(self.enemies) do
		if p:hasTreasure("wooden_ox") and p:getPile("wooden_ox"):length() > 0 then
			use.card = card
			if use.to then
				use.to:append(p)
			end
			return
		end
	end
end

sgs.ai_use_priority.ThXingxieCard = sgs.ai_use_priority.Slash + 0.1

sgs.ai_card_intention.ThXingxieCard = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if to:hasArmorEffect("silver_lion") and to:getArmor() and to:isWounded() then
		else
			sgs.updateIntention(from, to, 30)
		end
	end
end

--羽帛：出牌阶段限一次，你可以弃置一张黑色手牌并将一至两名角色的人物牌横置。
local thyubo_skill = {}
thyubo_skill.name = "thyubo"
table.insert(sgs.ai_skills, thyubo_skill)
thyubo_skill.getTurnUseCard = function(self)
	if self.player:canDiscard(self.player, "h") and not self.player:hasUsed("ThYuboCard") then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByUseValue(cards, true)
		for _, c in ipairs(cards) do
			if c:isBlack() and not self:isValuableCard(c) then
				return sgs.Card_Parse("@ThYuboCard=" .. c:getEffectiveId())
			end
		end
	end
end

sgs.ai_skill_use_func.ThYuboCard = function(card, use, self)
	local victims = self.room:getAlivePlayers()
	local target1 = self:findPlayerToChain(victims, true)
	if target1 then
		use.card = card
		if use.to then
			use.to:append(target1)
			victims:removeOne(target1)
			local target2 = self:findPlayerToChain(victims, true)
			if target2 then
				use.to:append(target2)
			end
		end
	end
end

sgs.ai_card_intention.ThYuboCard = 60
sgs.ai_use_priority.ThYuboCard = 5

function SmartAI:isGoodThQiongfaTarget(target)
	local current = self.room:getCurrent()
	if not current then return false end
	if current:getPhase() >= sgs.Player_Finish then
		current = self.room:findPlayer(current:getNextAlive(1, false):objectName())
	end
	local lingxians = self.room:findPlayersBySkillName("thqiongfa")
	local has_lingxian = false
	for _, s in sgs.qlist(lingxians) do
		if self:isFriend(s, current) then
			has_lingxian = true
			break
		end
	end
	if not has_lingxian then return false end
	if target:objectName() == current:objectName() then return true end
	if self:getEnemyNumBySeat(current, target, nil, true) == 0 then
		return true
	end
	return false
end

--穹法：人物牌横置的角色的结束阶段开始时，你可以令其选择一项：弃置由你指定的另一名角色的一张牌或令你摸一张牌。然后其重置其人物牌。
sgs.ai_skill_invoke.thqiongfa = function(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) and not self:isGoodChainPartner(target) then
		return true
	elseif not self:isFriend(target) and self:isGoodChainPartner(target) then
		return true
	end
	return false
end

sgs.ai_skill_playerchosen.thqiongfa = function(self, targets)
	local current = nil
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if p:getPhase() == sgs.Player_Finish then
			current = target
			break
		end
	end
	if not current then
		self.room:writeToConsole("No Qiongfa Source!")
		current = self.player
	end
	if self.player:objectName() == current:objectName() then
		local victim = self:findPlayerToDiscard("he", true, true, self.room:getOtherPlayers(current), false)
		if victim then
			local card_id = self:askForCardChosen(victim, "he", "thqiongfa", sgs.Card_MethodDiscard)
			if victim:hasEquip(sgs.Sanguosha:getCard(card_id)) or victim:getCardCount() < 3 then
				return victim
			end
		end
		return nil
	end
	if self:isFriend(current) then
		local vcitim = self:findPlayerToDiscard("he", true, true, self.room:getOtherPlayers(current), false)
		if victim then
			return victim
		else
			return targets:at(math.random(0, targets:length() - 1))
		end
	else
		self:sort(self.enemies, "handcard")
		for _, p in ipairs(self.enemies) do
			if not self:needToThrowArmor(p) and not (self:needKongcheng(p) and p:getHandcardNum() == 1) then
				return p
			end
		end
		return targets:at(math.random(0, targets:length() - 1))
	end
end

sgs.ai_skill_choice.thqiongfa = function(self, choices, data)
	local source = self.player:getTag("ThQiongfaSource"):toPlayer()
	local target = data:toPlayer()
	if self:isFriend(source) and not self:isEnemy(target) then
		return "cancel"
	end
	if self:isEnemy(source) and self:isFriend(target) then
		if self:getOverflow(target) > 1 then
			return "discard"
		end
		return "cancel"
	end
	if self:isFriend(source) and self:isEnemy(target) then
		local card_id = self:askForCardChosen(target, "he", "thqiongfa", sgs.Card_MethodDiscard)
		if target:hasEquip(sgs.Sanguosha:getCard(card_id)) or victim:getCardCount() < 3 then
			return "discard"
		end
		return "cancel"
	end
	return not self:isEnemy(source) and "cancel" or "discard"
end

--威德：摸牌阶段摸牌时，你可以放弃摸牌，改为观看牌堆顶的两张牌并交给一名角色，然后若该角色不为你且你已受伤，你可以获得一名其他角色的一张手牌。
sgs.ai_skill_invoke.thweide = true

sgs.ai_skill_playerchosen.thweide = function(self, targets)
	local ids = self.player:getTag("ThWeide"):toIntList()
	if not ids or ids:isEmpty() then
		return self:findPlayerToDiscard("h", false, false) or targets:at(math.random(0, targets:length() - 1))
	else
		local target, _ = sgs.ai_skill_askforyiji.nosyiji(self, sgs.QList2Table(ids))
		return target or self.player
	end
end

sgs.ai_playerchosen_intention.thweide = function(self, from, to)
	local ids = from:getTag("ThWeide"):toIntList()
	if ids and not ids:isEmpty() then
		sgs.updateIntention(from, to, -30)
	end
end

--诡卷：出牌阶段，你可以摸一张牌，然后展示之并选择一项：使用此牌，或失去1点体力。若你以此法使用了一张【杀】或装备牌，你不可以发动“诡卷”，直到回合结束。
local thguijuan_skill = {}
thguijuan_skill.name = "thguijuan"
table.insert(sgs.ai_skills, thguijuan_skill)
thguijuan_skill.getTurnUseCard = function(self)
	if not self.player:hasFlag("ForbidThGuijuan") then
		if self:getCardsNum("Peach") >= 1 or (self.player:getHp() <= 1 and self:getCardsNum("Analeptic") >= 1) or self.player:getHp() > 2 then
			return sgs.Card_Parse("@ThGuijuanCard=.")
		end
	end
end

sgs.ai_skill_use_func.ThGuijuanCard = function(card, use, self)
	use.card = card
end

sgs.ai_use_priority.ThGuijuanCard = 8

--诈诱：当其他角色使用的【杀】被你的【闪】抵消后，你可以摸一张牌，然后令该角色选择一项：对你使用一张无视距离的【杀】，或受到你对其造成的1点伤害。
sgs.ai_skill_invoke.thzhayou = function(self, data)
	local effect = data:toSlashEffect()
	return self:isEnemy(effect.from) or not self:damageIsEffective(effect.from, nil, self.player)
end

sgs.ai_skill_cardask["@thzhayou"] = function(self, _, __, target)
	if self:isFriend(target) and not self:damageIsEffective(self.player, nil, target) then
		return "."
	end
	return sgs.ai_skill_use.slash(self, "thzhayou")
end
