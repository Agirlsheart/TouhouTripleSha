--客星：准备阶段开始时，你可以从牌堆顶亮出X张牌（X为存活角色的数量，且至多为5），将其中任意数量的非锦囊牌以任意顺序置于牌堆底，然后将其余的牌置入弃牌堆。
sgs.ai_skill_invoke.thkexing = true

sgs.ai_skill_askforag.thkexing = function(self, card_ids)
	for _, id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		local typeId = card:getTypeId()
		local use = { to = sgs.SPlayerList() }
		self["use" .. sgs.ai_type_name[typeId + 1] .. "Card"](self, card, use)
		if not use.card then return id end
	end
	return -1
end

--神风：出牌阶段限一次，你可将两张相同颜色的牌交给一名体力值比你多的角色，然后该角色须对其距离为1的由你指定的一名角色造成1点伤害。
local thshenfeng_skill = {}
thshenfeng_skill.name = "thshenfeng"
table.insert(sgs.ai_skills, thshenfeng_skill)
thshenfeng_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("ThShenfengCard") and self.player:getCardCount() > 1 then
		return sgs.Card_Parse("@ThShenfengCard=.")
	end
end

sgs.ai_skill_use_func.ThShenfengCard = function(card, use, self)
	local distanceTo = function(from, to, has_horse)
		local range_fix = 0
		if to:objectName() == self.player:objectName() and has_horse then
			range_fix = range_fix - 1
		end
		return from:distanceTo(to, range_fix)
	end
	local getCardsValue = function(target, card1, card2)
		local value = 2
		if isCard("Peach", card1, target) then
			value = value + 5
		elseif self:isValuableCard(card1, target) then
			value = value + 2
		end
		if isCard("Peach", card2, target) then
			value = value + 5
		elseif self:isValuableCard(card2, target) then
			value = value + 2
		end
		return value
	end
	local getThShenfengValue = function(target, cards, has_horse)
		if self.player:getRole() == "rebel" and target:isLord() then
			for _, p in sgs.qlist(self.room:getOtherPlayers(target)) do
				if distanceTo(target, p, has_horse) == 1 and self:isEnemy(p) and p:getHp() == 1 and self:damageIsEffective(p, nil, target) and self:getAllPeachNum(p) == 0 then
					if not isCard("Peach", card1, target) and not isCard("Peach", card2, target) then
						local card_value = getCardsValue(target, cards[1], cards[2])
						return 7 - card_value
					end
				end
			end
		end
		for _, p in sgs.qlist(self.room:getOtherPlayers(target)) do
			if distanceTo(target, p, has_horse) == 1 and self:isEnemy(p) and p:getHp() == 1 and self:damageIsEffective(p, nil, target) and self:getAllPeachNum(p) == 0 then
				local value = 5
				if p:getRole() == "rebel" then value = value + 3 end
				local card_value = getCardsValue(target, cards[1], cards[2])
				local update = self:isFriend(target) and card_value or -card_value
				return value + update
			end
		end
		for _, p in sgs.qlist(self.room:getOtherPlayers(target)) do
			if distanceTo(target, p, has_horse) == 1 and self:isEnemy(p) and self:damageIsEffective(p, nil, target) then
				local value = 4
				local card_value = getCardsValue(target, cards[1], cards[2])
				local update = self:isFriend(target) and card_value or -card_value
				return value + update
			end
		end
		for _, p in sgs.qlist(self.room:getOtherPlayers(target)) do
			if distanceTo(target, p, has_horse) == 1 and not self:damageIsEffective(p, nil, target) then
				local value = 0
				local card_value = getCardsValue(target, cards[1], cards[2])
				local update = self:isFriend(target) and card_value or -card_value
				return value + update
			end
		end
		return -100
	end
	local comparePlayersByShenfengValue = function(a, b)
		if a.v == b.v then
			return table.indexOf(a) < table.indexOf(b)
		else
			return a.v > b.v
		end
	end
	local compareCardsByShenfengValue = function(a, b)
		if a.target and not b.target then
			return true
		elseif b.target and not a.target then
			return false
		elseif a.target and b.target then
			return a.value > b.value
		else
			return true
		end
	end
	local cards = sgs.QList2Table(self.player:getCards("he"))
	local can_use = {}
	for _, c in ipairs(cards) do
		for _, c2 in ipairs(cards) do
			if c:getEffectiveId() == c2:getEffectiveId() then
				continue
			end
			if c:sameColorWith(c2) then
				table.insert(can_use, { cards = {c, c2} })
			end
		end
	end
	if #can_use == 0 then return end
	local targets = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	self:sort(targets, "handcard")
	for index, sub_t in ipairs(can_use) do
		local cs = sub_t.cards
		local sort_table = {}
		local has_horse = self.player:getDefensiveHorse() and (cs[1]:getEffectiveId() == self.player:getDefensiveHorse():getEffectiveId() or cs[2]:getEffectiveId() == self.player:getDefensiveHorse():getEffectiveId())
		for _, target in ipairs(targets) do
			local value = getThShenfengValue(target, cs, has_horse)
			table.insert(sort_table, { p = target, v = value })
		end
		table.sort(sort_table, comparePlayersByShenfengValue)
		if sort_table[1].v > 0 then
			can_use[index].target = sort_table[1].p
			can_use[index].value = sort_table[1].v
		end
	end
	table.sort(can_use, compareCardsByShenfengValue)
	if can_use[1] and can_use[1].target then
		use.card = sgs.Card_Parse(("@ThShenfengCard=%d+%d"):format(can_use[1].cards[1]:getEffectiveId(), can_use[1].cards[2]:getEffectiveId()))
		if use.to then
			use.to:append(can_use[1].target)
		end
	end
end

sgs.ai_skill_playerchosen.thshenfeng = function(self, targets)
	local source = self.player:getTag("ThShenfengTarget"):toPlayer()
	targets = sgs.QList2Table(targets)
	self:sort(targets, "hp")
	local null = nil
	for _, p in ipairs(targets) do
		if self:isEnemy(p) and self:damageIsEffective(p, nil, source) then
			return p
		end
		if not self:damageIsEffective(p, nil, source) then
			null = p
		end
	end
	return null or targets[#targets]
end

sgs.ai_card_intention.ThShenfengCard = -40

--开海：每当你失去最后的手牌时，可从牌堆底摸一张牌。
sgs.ai_skill_invoke.thkaihai = true

--天宝：锁定技，准备阶段和结束阶段开始时，你须弃置你人物牌上的灵宝牌，然后将一张随机灵宝牌置于你的人物牌上，并获得相应技能直到失去该灵宝牌。
--无

--仙命：锁定技，你须弃置即将进入你装备区的装备牌，每当你以此法弃置牌后，你须发动“天宝”。
--无

--巫道：锁定技，游戏开始时，你失去4点体力。准备阶段开始时，你失去或回复体力至X点（X为你已损失的体力值）。
--smart-ai.lua getBestHp

--幻君：锁定技，你的方块【闪】均视为【碎月绮斗】，你手牌中的防具牌均视为【酒】，你获得即将进入你装备区的防具牌。
--smart-ai.lua isCard

--孤高：出牌阶段限一次，你可以与一名其他角色拼点，赢的角色对没有赢的角色造成1点伤害；“千狱”发动后，防止你因没有赢而受到的伤害；“皇仪”发动后，若你赢，视为你此阶段没有发动“孤高”。
local thgugao_skill = {}
thgugao_skill.name = "thgugao"
table.insert(sgs.ai_skills, thgugao_skill)
thgugao_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("ThGugaoCard") and not self.player:isKongcheng() then
		return sgs.Card_Parse("@ThGugaoCard=.")
	end
end

sgs.ai_skill_use_func.ThGugaoCard = function(card, use, self)
	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard()
	if not max_card then return end
	local max_point = max_card:getNumber()

	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if not (enemy:hasSkill("ikjingyou") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() then
			local enemy_max_card = self:getMaxCard(enemy)
			local enemy_max_point = enemy_max_card and enemy_max_card:getNumber() or 14
			if max_point > enemy_max_point then
				self.thgugao_card = max_card:getId()
				use.card = card
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if not (enemy:hasSkill("ikjingyou") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() then
			if max_point >= 10 then
				self.thgugao_card = max_card:getId()
				use.card = card
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	end
end

function sgs.ai_skill_pindian.thgugao(minusecard, self, requestor)
	if requestor:getHandcardNum() == 1 then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)
		return cards[1]
	end
	return self:getMaxCard()
end

sgs.ai_cardneed.thgugao = sgs.ai_cardneed.bignumber

sgs.ai_use_priority.ThGugaoCard = 7.2
sgs.ai_card_intention.ThGugaoCard = 60
sgs.ai_use_value.ThGugaoCard = 8.5

--千狱：觉醒技，一名角色的回合结束后，若你的体力值为1，你须将体力上限减少至1点，并获得技能“狂魔”，然后进行一个额外的回合。
--无

--狂魔：锁定技，专属技，每当你对一名角色造成除【杀】以外的1点伤害后，你增加1点体力上限，然后回复1点体力。
sgs.ai_cardneed.thkuangmo = function(to, card, self)
	return not card:isKindOf("Slash") and sgs.dynamic_value.damage_card[card:getClassName()]
end

--皇仪：觉醒技，“千狱”发动后，准备阶段开始时，若你的体力上限大于4点，你须减少至4点，并摸等同于减少数量的牌，然后回复1点体力，并失去技能“狂魔”。
--无

--肃狐：摸牌阶段开始时，你可放弃摸牌，改为将牌堆顶的一张牌面朝上置于你的人物牌上，称为“面”，然后进行一个额外的出牌阶段。
sgs.ai_skill_invoke.thsuhu = function(self, data)
	return not (self.player:getPile("faces"):length() > 0 and self.player:getHp() == 1 and self:getAllPeachNum() == 0 and self:getCardsNum("Analeptic") == 0)
end

--忿狼：锁定技，专属技，每当你因受到伤害而扣减1点体力后，回复1点体力并摸两张牌。
--无

--乐狮：出牌阶段限一次，你可以选择一种牌的类别并摸两张牌，视为你此阶段没有使用过此类别的牌。
getLeshiString = function(self)
	if self.player:hasSkill("thjingyuan") then
		local cards = self.player:getHandcards()
		for _, id in sgs.qlist(getWoodenOxPile(self.player)) do
			cards:prepend(sgs.Sanguosha:getCard(id))
		end
		cards = sgs.QList2Table(cards)
	
		local turnUse = {}
		local slashAvail = self.slashAvail
		for _, skill in ipairs(sgs.ai_skills) do
			if self.player:hasSkill(skill.name) or (skill.name == "shuangxiong" and self.player:hasFlag("shuangxiong")) then
				local skill_card = skill.getTurnUseCard(self, #cards == 0)
				if skill_card then table.insert(cards, skill_card) end
			end
		end
		self:sortByUseValue(cards)

		for _, card in ipairs(cards) do
			local dummy_use = { isDummy = true }

			local typeId = card:getTypeId()
			self["use" .. sgs.ai_type_name[typeId + 1] .. "Card"](self, card, dummy_use)

			if dummy_use.card then
				if dummy_use.card:isKindOf("Slash") then
					if slashAvail > 0 then
						slashAvail = slashAvail - 1
						table.insert(turnUse, dummy_use.card)
					elseif dummy_use.card:hasFlag("AIGlobal_KillOff") then table.insert(turnUse, dummy_use.card) end
				else
					table.insert(turnUse, dummy_use.card)
				end
			end
		end

		type_table = { basic = "BasicCard", trick = "TrickCard", equip = "EquipCard" }
		for _, card in ipairs(turnUse) do
			if card:isKindOf("SkillCard") then
				continue
			end
			local use = { isDummy = true }
			local typeId = card:getTypeId()
			self["use" .. sgs.ai_type_name[typeId + 1] .. "Card"](self, card, use)
			if not use.card then continue end
			if self.player:hasFlag("thjingyuan_" .. type_table[card:getType()]) then
				return type_table[card:getType()]
			end
		end
		for _, str in pairs(type_table) do
			if self.player:hasFlag("thjingyuan_" .. str) then
				return str
			end
		end
		return "BasicCard"
	else
		return "BasicCard"
	end
end

local thleshi_skill = {}
thleshi_skill.name = "thleshi"
table.insert(sgs.ai_skills, thleshi_skill)
thleshi_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("ThLeshiCard") and not self:doNotDraw() then
		return sgs.Card_Parse("@ThLeshiCard=.")
	end
end

sgs.ai_skill_use_func.ThLeshiCard = function(card, use, self)
	use.card = card
end

sgs.ai_skill_choice.thleshi = function(self, choices, data)
	return getLeshiString(self)
end

--忧狸：锁定技，你的手牌上限始终为4。每当你的手牌数变化后，若大于四张，须将多余的作为“面”置于你的人物牌上。每当你的“面”的数量不小于两张时，须弃置两张“面”并失去1点体力。
function SmartAI:doNotDraw(target, num, TrickCard)
	target = target or self.player
	num = num or 2
	if TrickCard then
		if TrickCard:isVirtualCard() then
			for _, id in sgs.qlist(TrickCard:getSubcards()) do
				if target:handCards():contains(id) then
					num = num - 1
				end
			end
		else
			if target:handCards():contains(TrickCard:getEffectiveId()) then
				num = num - 1
			end
		end
	end
	if target:hasSkill("thyouli") then return target:getHandcardNum() + num > 4 end
	return false
end

--惊猿：锁定技，出牌阶段，每种类别的牌你只能使用一张。
--无

--彷魂：摸牌阶段结束时，你可以弃置一张牌，然后你计算与所有其他角色的距离始终为1，且你的【杀】均视为【赤雾锁魂】，直到回合结束。
sgs.ai_skill_invoke.thpanghun = function(self, data)
	for _, p in ipairs(self.friends_noself) do
		if getCardsNum("NatureSlash", p, self.player) > 0 or getCardsNum("Fan", p, self.player) > 0 then
			return true
		end
	end
	for _, p in ipairs(self.enemies) do
		if self:getDangerousCard(p) or self:getValuableCard(p) then
			return true
		end
	end
	local black_trick = false
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:isKindOf("TrickCard") and c:isBlack() then
			black_trick = true
			break
		end
	end
	if not black_trick then
		for _, c in sgs.qlist(self.player:getHandcards()) do
			if c:isKindOf("Slash") and c:isBlack() then
				black_trick = true
				break
			end
		end
		if black_trick then
			for _, p in ipairs(self.friends) do
				if (p:containsTrick("indulgence") and p:getOverflow() > 0) or p:containsTrick("supply_shortage") then
					return true
				end
			end
		end
	end
	local snatch = self:getCard("Snatch")
	if snatch then
		snatch = sgs.cloneCard("snatch")
		local use = { isDummy = true }
		self:useCardSnatchOrDismantlement(snatch, use)
		if not use.card then
			snatch:setSkillName("thpanghun")
			self:useCardSnatchOrDismantlement(snatch, use)
			if use.card then
				return true
			end
		end
	end
	local slash = self:getCard("Slash")
	if slash then
		slash = sgs.cloneCard("slash")
		local use = { isDummy = true }
		self:useCardSlash(slash, use)
		if not use.card then
			return true
		end
	end
	return false
end

sgs.ai_skill_cardask["@thpanghun"] = function(self, data, pattern)
	if sgs.ai_skill_invoke.thpanghun(self, data) then
		local ret = self:askForDiscard("thpanghun", 1, 1, false, true)
		if self:isValuableCard(sgs.Sanguosha:getCard(ret[1])) then
			return "."
		end
		return "$" .. ret[1]
	end
	return "."
end

--镜悟：出牌阶段限一次，你可以展示一张黑色锦囊牌，然后弃置场上的一张牌。
local thjingwu_skill = {}
thjingwu_skill.name = "thjingwu"
table.insert(sgs.ai_skills, thjingwu_skill)
thjingwu_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("ThJingwuCard") then
		for _, c in sgs.qlist(self.player:getHandcards()) do
			if c:isKindOf("TrickCard") and c:isBlack() then
				return sgs.Card_Parse("@ThJingwuCard=" .. c:getEffectiveId())
			end
		end
	end
end

sgs.ai_skill_use_func.ThJingwuCard = function(card, use, self)
	local target = self:findPlayerToDiscard("ej", true, true, nil)
	if not target then return end
	use.card = card
	if use.to then
		use.to:append(target)
	end
end

sgs.ai_use_priority.ThJingwuCard = 9.5
sgs.ai_use_value.ThJingwuCard = sgs.ai_use_priority.Dismantlement
sgs.ai_choicemade_filter.cardChosen.thjingwu = sgs.ai_choicemade_filter.cardChosen.snatch

--轮狱：连舞技，当一张你不为其使用的目标（或之一）的牌置入弃牌堆时，你将这张牌置于牌堆顶，然后此阶段结束时你从牌堆底摸一张牌。
sgs.ai_skill_invoke.thlunyu = function(self, data)
	local str = data:toString()
	local str_t = str:split(":")
	local id = str_t[2]
	local card = sgs.Sanguosha:getEngineCard(id)
	local nextAlive = self.player
	repeat
		nextAlive = self.room:findPlayer(nextAlive:getNextAlive(1, false):objectName())
		if nextAlive:objectName() == self.player:objectName() then
			if not nextAlive:faceUp() then
				nextAlive = self.room:findPlayer(nextAlive:getNextAlive(1, false):objectName())
			end
			break
		end
	until nextAlive:faceUp()

	local willUseExNihilo, willRecast
	if self:getCardsNum("ExNihilo") > 0 then
		local ex_nihilo = self:getCard("ExNihilo")
		if ex_nihilo then
			local dummy_use = { isDummy = true }
			self:useTrickCard(ex_nihilo, dummy_use)
			if dummy_use.card then willUseExNihilo = true end
		end
	end
	if self:getCardsNum("IronChain") > 0 then
		local iron_chain = self:getCard("IronChain")
		if iron_chain then
			local dummy_use = { to = sgs.SPlayerList(), isDummy = true, canRecast = true }
			self:useTrickCard(iron_chain, dummy_use)
			if dummy_use.card and dummy_use.to:isEmpty() then willRecast = true end
		end
	end
	if (willUseExNihilo or willRecast) and self.player:getPhase() == sgs.Player_Play then
		if card:isKindOf("Peach") then
			return true
		end
		if card:isKindOf("TrickCard") or card:isKindOf("Indulgence") or card:isKindOf("SupplyShortage") then
			local dummy_use = { isDummy = true }
			self:useTrickCard(card, dummy_use)
			if dummy_use.card then
				return true
			end
		end
		if card:isKindOf("Jink") and self:getCardsNum("Jink") == 0 then
			return true
		end
		if card:isKindOf("Nullification") and self:getCardsNum("Nullification") == 0 then
			return true
		end
		if card:isKindOf("Slash") and self:slashIsAvailable() then
			local dummy_use = { isDummy = true }
			self:useBasicCard(card, dummy_use)
			if dummy_use.card then
				return true
			end
		end
	end

	local hasLightning, hasIndulgence, hasSupplyShortage, hasPurpleSong
	local tricks = nextAlive:getJudgingArea()
	if not tricks:isEmpty() and not nextAlive:containsTrick("YanxiaoCard") then
		local trick = tricks:at(tricks:length() - 1)
		if self:hasTrickEffective(trick, nextAlive) then
			if trick:isKindOf("Lightning") then hasLightning = true
			elseif trick:isKindOf("Indulgence") then hasIndulgence = true
			elseif trick:isKindOf("SupplyShortage") then hasSupplyShortage = true
			elseif trick:isKindOf("PurpleSong") then hasSupplyShortage = true
			end
		end
	end

	if nextAlive:hasSkill("luoshen") then
		return self:isEnemy(nextAlive) == card:isRed()
	end
	if nextAlive:hasSkill("yinghun") then
		return self:isFriend(nextAlive)
	end
	if hasLightning and not (nextAlive:hasSkill("qiaobian") and nextAlive:getHandcardNum() > 0) then
		if self:isEnemy(nextAlive) == ((card:getSuit() == sgs.Card_Spade or (card:getSuit() == sgs.Card_Haert and not nextAlive:hasSkill("ikchiqiu") and nextAlive:hasSkill("thanyue"))) and card:getNumber() >= 2 and (card:getNumber() <= 9 or nextAlive:hasSkill("thjiuzhang"))) then
			return true
		end
	end
	if hasIndulgence then
		if self:isFriend(nextAlive) == (card:getSuit() == sgs.Card_Heart or (card:getSuit() == sgs.Card_Spade and nextAlive:hasSkill("ikchiqiu"))) then
			return true
		end
	end
	if hasSupplyShortage and not (nextAlive:hasSkill("qiaobian") and nextAlive:getHandcardNum() > 0) then
		if self:isFriend(nextAlive) == (card:getSuit() == sgs.Card_Club or (card:getSuit() == sgs.Card_Diamond and nextAlive:hasSkill("ikmohua"))) then
			return true
		end
	end
	if hasPurpleSong then
		if self:isEnemy(nextAlive) == (card:getSuit() == sgs.Card_Diamond and nextAlive:hasSkill("ikmohua")) then
			return true
		end
	end
	return false
end

--返魂：专属技，当你处于濒死状态时，你可获得1枚“桜咲”标记且体力回复至1点，然后将你的人物牌翻至正面朝上并重置之。
sgs.ai_skill_invoke.thfanhun = function(self, data)
	local dying = data:toDying()
	local damage = dying.damage
	if self.player:getMark("@yingxiao") < 3 or not self.player:hasSkill("thmanxiao") then
		return true
	end
	if self:getAllPeachNum() == 0 and self:getCardsNum("Analeptic") == 0 then
		return not (damage and damage.from and self.player:getRole() == "rebel" and self:isFriend(damage.from))
	end
	return false
end

--诱殇：专属技，每当你的非黑桃牌对目标角色造成伤害时，你可以防止此伤害，并令该角色减少1点体力上限，然后获得1枚“桜咲”标记。
sgs.ai_skill_invoke.thyoushang = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.to) then
		return false
	elseif self:isEnemy(damage.to) then
		if damage.damage > 1 then return false end
		if damage.to:isWounded() then return false end
		if (self.player:getMark("@yingxiao") < 3 or not self.player:hasSkill("thmanxiao")) and self:objectiveLevel(damage.to) > 3 then
			return true
		end
		if self.player:getMark("@yingxiao") == 3 and #self.friends_noself > 0
				and self:isWeak() and self:getAllPeachNum() == 0 and self:getCardsNum("Analeptic") == 0
				and self:objectiveLevel(damage.to) == 5 then
			return true
		end
	end
	return false
end

sgs.ai_choicemade_filter.skillInvoke.thyoushang = function(self, player, promptlist)
	if promptlist[#promptlist] == "yes" then
		local target = player:getTag("ThYoushangData"):toDamage().to
		if target then
			sgs.updateIntention(player, target, 120)
		end
	end
end

--幽雅：每当你受到伤害时，你可以弃置一张牌并令一至X名角色选择一项：打出一张【闪】；或令你获得其一张牌。（X为你的“桜咲”标记的数量）
sgs.ai_skill_use["@@thyouya"] = function(self, prompt, method)
	local ret = self:askForDiscard("thyouya", 1, 1, false, true)
	if self:isValuableCard(sgs.Sanguosha:getCard(ret[1])) then
		return "."
	end
	card_str = "@ThYouyaCard=" .. ret[1]
	local to_table = {}
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isNude() and (getCardsNum("Jink", enemy, self.player) < 1 or sgs.card_lack[enemy:objectName()]["Jink"] == 1) then
			table.insert(to_table, enemy:objectName())
			if #to_table >= self.player:getMark("@yingxiao") then
				return card_str .. "->" .. table.concat(to_table, "+")
			end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isNude() and not table.contains(to_table, enemy:objectName()) then
			table.insert(to_table, enemy:objectName())
			if #to_table >= self.player:getMark("@yingxiao") then
				return card_str .. "->" .. table.concat(to_table, "+")
			end
		end
	end
	return "."
end

sgs.ai_skill_cardask["@thyouya-jink"] = function(self, data, pattern, target)
	if not self.player:isNude() and not self:isFriend(target) then
		return self:getCardId("Jink")
	end
	return "."
end

sgs.ai_card_intention.ThYouyaCard = 50

--满咲：锁定技，你每拥有1枚“桜咲”标记，你的手牌上限便+1。当你拥有4枚“桜咲”标记时，你立即死亡。
--无

--尽戮：出牌阶段限一次，你可以弃置一张手牌并指定一名其他角色，获得其全部的牌。此出牌阶段结束时，你须交给该角色等同于其体力值数量的牌，且结束阶段开始时将你的人物牌翻面。
local thjinlu_skill = {}
thjinlu_skill.name = "thjinlu"
table.insert(sgs.ai_skills, thjinlu_skill)
thjinlu_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ThJinluCard") or self.player:isNude() then return end
	local card_id
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	local lightning = self:getCard("Lightning")

	if self:needToThrowArmor() then
		card_id = self.player:getArmor():getId()
	elseif self.player:getHandcardNum() > self.player:getHp() then
		if lightning and not self:willUseLightning(lightning) then
			card_id = lightning:getEffectiveId()
		else
			for _, acard in ipairs(cards) do
				if (acard:isKindOf("BasicCard") or acard:isKindOf("EquipCard") or acard:isKindOf("AmazingGrace"))
					and not acard:isKindOf("Peach") then
					card_id = acard:getEffectiveId()
					break
				end
			end
		end
	end
	if not card_id then
		if lightning and not self:willUseLightning(lightning) then
			card_id = lightning:getEffectiveId()
		else
			for _, acard in ipairs(cards) do
				if (acard:isKindOf("BasicCard") or acard:isKindOf("EquipCard") or acard:isKindOf("AmazingGrace"))
					and not acard:isKindOf("Peach") then
					card_id = acard:getEffectiveId()
					break
				end
			end
		end
	end
	if not card_id then
		return nil
	else
		return sgs.Card_Parse("@ThJinluCard=" .. card_id)
	end
end

sgs.ai_skill_use_func.ThJinluCard = function(card, use, self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)

	if not self.player:hasUsed("ThJinluCard") then
		self:sort(self.enemies, "cardcount")
		self.enemies = sgs.reverse(self.enemies)
		local target
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isNude() and not enemy:hasSkill("ikjingyou") then
				if (self:needKongcheng(enemy) and not enemy:isKongcheng() and self:damageMinusHp(self, enemy, 1) > 0)
					or (enemy:getHp() < 3 and self:damageMinusHp(self, enemy, 0) > 0 and enemy:getCardCount() > 0)
					or (enemy:getCardCount() >= enemy:getHp() and enemy:getHp() > 2 and self:damageMinusHp(self, enemy, 0) >= -1)
					or (enemy:getCardCount() - enemy:getHp() > 2) then
					target = enemy
					break
				end
			end
		end
		if not self.player:faceUp() and not target then
			for _, enemy in ipairs(self.enemies) do
				if not enemy:isNude() then
					if enemy:getCardCount() >= enemy:getHp() then
						target = enemy
						break
					end
				end
			end
		end

		if not target and (self:hasCrossbowEffect() or self:getCardsNum("Crossbow") > 0) then
			local slash = self:getCard("Slash") or sgs.cloneCard("slash")
			for _, enemy in ipairs(self.enemies) do
				if self:slashIsEffective(slash, enemy) and self.player:distanceTo(enemy) == 1
					and not enemy:hasSkills("thzhehui|ikzhichi|fankui|nosfankui|ganglie|vsganglie|nosganglie|enyuan|thfusheng|langgu|guixin|ikjingyou")
					and self:getCardsNum("Slash") + getKnownCard(enemy, self.player, "Slash") >= 3 then
					target = enemy
					break
				end
			end
		end

		if target then
			use.card = card
			if use.to then use.to:append(target) end
		end
	end
end

function SmartAI:isThJinluTarget(player, drawCardNum)
	player = player or self.player
	drawCardNum = drawCardNum or 1
	if type(player) == "table" then
		if #player == 0 then return false end
		for _, ap in ipairs(player) do
			if self:isThJinluTarget(ap, drawCardNum) then return true end
		end
		return false
	end

	local handCardNum = player:getHandcardNum() + drawCardNum

	local sb_diaochan = self.room:findPlayerBySkillName("thjinlu")
	local thjinlu = sb_diaochan and not sb_diaochan:hasUsed("ThJinluCard") and not self:isFriend(sb_diaochan)

	if not thjinlu then return false end

	if sb_diaochan:getPhase() == sgs.Player_Play then
		if (handCardNum - player:getHp() >= 2)
			or (handCardNum > 0 and handCardNum - player:getHp() >= -1 and not sb_diaochan:faceUp()) then
			return true
		end
	else
		if sb_diaochan:faceUp() and not self:willSkipPlayPhase(sb_diaochan)
			and self:playerGetRound(player) > self:playerGetRound(sb_diaochan) and handCardNum >= player:getHp() + 2 then
			return true
		end
	end

	return false
end

sgs.ai_skill_discard.thjinlu = function(self, discard_num, min_num, optional, include_equip)
	local to_discard = {}

	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
	local card_ids = {}
	for _, card in ipairs(cards) do
		table.insert(card_ids, card:getEffectiveId())
	end

	local temp = table.copyFrom(card_ids)
	for i = 1, #temp, 1 do
		local card = sgs.Sanguosha:getCard(temp[i])
		if self.player:getArmor() and temp[i] == self.player:getArmor():getEffectiveId() and self:needToThrowArmor() then
			table.insert(to_discard, temp[i])
			table.removeOne(card_ids, temp[i])
			if #to_discard == discard_num then
				return to_discard
			end
		end
	end

	temp = table.copyFrom(card_ids)

	for i = 1, #card_ids, 1 do
		local card = sgs.Sanguosha:getCard(card_ids[i])
		table.insert(to_discard, card_ids[i])
		if #to_discard == discard_num then
			return to_discard
		end
	end

	if #to_discard < discard_num then return {} end
end

sgs.ai_use_value.ThJinluCard = 8.5
sgs.ai_use_priority.ThJinluCard = 6
sgs.ai_card_intention.ThJinluCard = 80

--狂戾：每当你的人物牌翻面时，你可以摸一张牌。
--thicket-ai.lua toTurnOver

--愈心：摸牌阶段开始时，你可以放弃摸牌，改为从牌堆顶亮出两张牌并获得之，若这两张牌均为红色，你可以指定一名角色并令其回复1点体力。
sgs.ai_skill_invoke.thyuxin = function(self, data)
	local target = self:findPlayerToRecover()
	if target then
		return true
	end
	return false
end

sgs.ai_skill_playerchosen.thyuxin = function(self, targets)
	return self:findPlayerToRecover(1, targets)
end

sgs.ai_playerchosen_intention.thyuxin = -100

--疮心：出牌阶段开始时，你可弃置X张牌，然后选择X项：1. 获得技能“灵视”，直到回合结束2. 获得技能“闭月”，直到回合结束
sgs.ai_skill_use["@@thchuangxin"] = function(self, prompt, method)
	local needGongxin = false
	local useBiyueValue, useGongxinValue = false, false
	if self:isWeak() then
		useBiyueValue = true
	end
	for _, p in ipairs(self.enemies) do
		local flag = ("%s_%s_%s"):format("visible", self.player:objectName(), p:objectName())
		local has_peach = false
		for _, c in sgs.qlist(p:getHandcards()) do
			if c:hasFlag("visible") or c:hasFlag(flag) then
				if isCard("Peach", c, p) and c:getSuit() == sgs.Card_Heart then
					has_peach = true
					break
				end
			end
		end
		if has_peach then
			needGongxin = true
			useGongxinValue = true
			break
		end
	end
	if self.player:getHandcardNum() > 3 then
		needGongxin = true
	end
	local needNum = 1
	if needGongxin then
		needNum = needNum + 1
	end
	local valueNum = 0
	if useBiyueValue then
		valueNum = valueNum + 1
	end
	if useGongxinValue then
		valueNum = valueNum + 1
	end
	if needNum < valueNum then
		self.room:writeToConsole("ThChuangxin F**k Dog!!!")
		needNum = valueNum
	end
	local use_cards = {}
	local ret1 = self:askForDiscard("thchuangxin", 1, 1, false, true)
	if #ret1 == 1 then
		local id1 = ret1[1]
		local card1 = sgs.Sanguosha:getCard(id1)
		if valueNum > 0 and not isCard("Peach", card1, self.player) then
			table.insert(use_cards, id1)
		elseif not self:isValuableCard(card1) then
			local type = card1:getTypeId()
			local use = { isDummy = true }
			self["use" .. sgs.ai_type_name[type + 1] .. "Card"](self, card1, use)
			if not use.card then
				table.insert(use_cards, id1)
			end
		end
		if #use_cards == 1 and needNum > 1 then
			local ret2 = self:askForDiscard("thchuangxin", 2, 2, false, true)
			if #ret2 == 2 then
				local id2 = ret2[2]
				local card2 = sgs.Sanguosha:getCard(id2)
				if valueNum > 1 and not isCard("Peach", card2, self.player) then
					table.insert(use_cards, id2)
				elseif not self:isValuableCard(card2) then
					local type = card2:getTypeId()
					local use = { isDummy = true }
					self["use" .. sgs.ai_type_name[type + 1] .. "Card"](self, card2, use)
					if not use.card then
						table.insert(use_cards, id2)
					end
				end
			end
		end
	end
	if #use_cards > 0 then
		if #use_cards == 1 then
			sgs.thchuangxin_choice = "ikbiyue"
			if not useBiyueValue and useGongxinValue then
				sgs.thchuangxin_choice = "iklingshi"
			end
		end
		return "@ThChuangxinCard=" .. table.concat(use_cards, "+")
	end
	return "."
end

sgs.ai_skill_choice.thchuangxin = function(self, choices, data)
	return sgs.thchuangxin_choice
end

--天心：你的回合开始时，你可弃置X张牌，然后选择X项：1. 获得技能“预悉”，直到回合结束2. 获得技能“天妒”，直到回合结束
sgs.ai_skill_use["@@thtianxin"] = function(self, prompt, method)
	local needGuanxing, needTiandu = false
	if self.player:containsTrick("indulgence") or self.player:containsTrick("supply_shortage") or self.player:containsTrick("lightning") then
		needGuanxing = true
	end
	if self.player:containsTrick("supply_shortage") and self.player:getJudgingArea():length() > 1 then
		needTiandu = true
	end
	if self:getOverflow() > 0 then
		needGuanxing = true
	end
	local weak = self:findPlayerToRecover()
	if weak and self:isFriend(weak) and self.player:getHandcardNum() > 1 then
		needGuanxing = true
	end
	local needNum = 0
	if needGuanxing then
		needNum = needNum + 1
	end
	if needTiandu then
		needNum = needNum + 1
	end
	if needNum == 0 then return "." end
	local use_cards = {}
	local ret1 = self:askForDiscard("thtianxin", 1, 1, false, true)
	if #ret1 == 1 then
		local id1 = ret1[1]
		if not isCard("Peach", id1, self.player) or self:willSkipPlayPhase() then
			table.insert(use_cards, id1)
		end
		if #use_cards == 1 and needNum > 1 then
			local ret2 = self:askForDiscard("thtianxin", 2, 2, false, true)
			if #ret2 == 2 then
				local id2 = ret2[2]
				if not isCard("Peach", id2, self.player) or self:willSkipPlayPhase() then
					table.insert(use_cards, id2)
				end
			end
		end
	end
	if #use_cards > 0 then
		if #use_cards == 1 then
			sgs.thtianxin_choice = "ikyuxi"
			if not needGuanxing and needTiandu then
				sgs.thtianxin_choice = "iktiandu"
			end
		end
		return "@ThTianxinCard=" .. table.concat(use_cards, "+")
	end
	return "."
end

sgs.ai_skill_choice.thtianxin = function(self, choices, data)
	return sgs.thtianxin_choice
end
