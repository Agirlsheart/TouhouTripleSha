-- translation for SP Package

return {
	["sp"] = "SP包",
	
	["#sp004"] = "日出国之贤兽",
	["sp004"] = "上白泽慧音",
	["designer:sp004"] = "幻桜落 | Codeby:Slob",
	["illustrator:sp004"] = "正体不明",
	["cv:sp004"] = "暂无",
	["thchuangshi"] = "创史",
	[":thchuangshi"] = "你可以抵消其他角色使用的一张【过河拆桥】、【借刀杀人】或【决斗】对你攻击范围内的一名角色的效果，视为使用锦囊牌的角色对你使用了一张【杀】（此【杀】不计入每阶段的使用限制）。",
	["thgaotian"] = "高天",
	[":thgaotian"] = "每当你需要使用或打出一张【闪】时，你可观看牌堆顶的X张牌并获得其中至多一张红色牌（X为存活角色的数量，且至多为4），你可以至多弃置一张牌并获得其中一张相同颜色的牌，然后将其余的任意数量的牌以任意顺序置于牌堆顶或置入弃牌堆。",
	["thgaotian:get"] = "获得",
	["thgaotian:discard"] = "置入弃牌堆",
--[[
	["#yangxiu"] = "恃才放旷",
	["yangxiu"] = "杨修",
	["illustrator:yangxiu"] = "张可",
	["jilei"] = "鸡肋",
	["danlao"] = "啖酪",
	[":jilei"] = "每当你受到伤害时，你可以说出一种牌的类别，令伤害来源不能使用、打出或弃置其此类别的手牌，直到回合结束。\
◆你对角色发动【鸡肋】说出一种牌的类别，该角色在执行技能效果或在弃牌阶段弃置手牌时，非该类别的手牌数不足，则该角色须弃置全部非该类别的手牌并展示剩余的该类别的手牌。",
	[":danlao"] = "当一张锦囊牌指定包括你在内的多名目标后，你可以摸一张牌，若如此做，此锦囊牌对你无效。",
	["$jilei"] = "食之无味，弃之可惜",
	["$danlao"] = "一人一口，分而食之",
	["~yangxiu"] = "恃才傲物，方有此命",
	["cv:yangxiu"] = "幻象迷宫",
	["#DanlaoAvoid"] = "%from 发动【%arg2】， %arg 对他无效",
	["#Jilei"] = "%from 鸡肋了 %to 的 %arg",
	["#JileiClear"] = "%from 的鸡肋效果消失",

	["#sp_diaochan"] = "绝世的舞姬",
	["sp_diaochan"] = "SP貂蝉",
	["&sp_diaochan"] = "貂蝉",
	["illustrator:sp_diaochan"] = "巴萨小马",
	["tuoqiao"] = "脱壳",
	[":tuoqiao"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP貂蝉",
	["SP-Diaochan"] = "SP貂蝉",

	["#sp_sunshangxiang"] = "梦醉良缘",
	["sp_sunshangxiang"] = "SP孙尚香",
	["&sp_sunshangxiang"] = "孙尚香",
	["illustrator:sp_sunshangxiang"] = "木美人",
	["chujia"] = "出嫁",
	[":chujia"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP孙尚香，势力为蜀",
	["cv:sp_sunshangxiang"] = "官方，背碗卤粉",

	["#sp_guanyu"] = "汉寿亭侯",
	["sp_guanyu"] = "SP关羽",
	["&sp_guanyu"] = "关羽",
	["illustrator:sp_guanyu"] = "LiuHeng",
	["danji"] = "单骑",
	[":danji"] = "<b>觉醒技</b>，回合开始阶段，若你的手牌数大于你当前的体力值，且本局主公为曹操时，你须减1点体力上限并永久获得技能“马术”。",
	["#DanjiWake"] = "%from 的手牌数(%arg)多于体力值(%arg2)，且本局主公为曹操，达到【单骑】的觉醒条件",

	["#sp_caiwenji"] = "金璧之才",
	["sp_caiwenji"] = "SP蔡文姬",
	["&sp_caiwenji"] = "蔡文姬",
	["illustrator:sp_caiwenji"] = "木美人",
	["guixiang"] = "归乡",
	[":guixiang"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP蔡文姬，势力为魏",
	["cv:sp_caiwenji"] = "呼呼",
	
	["#sp_jiaxu"] = "算无遗策",
	["sp_jiaxu"] = "SP贾诩",
	["&sp_jiaxu"] = "贾诩",
	["illustrator:sp_jiaxu"] = "雪君S",
	["guiwei"] = "归魏",
	[":guiwei"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP贾诩，势力为魏",
	["cv:sp_jiaxu"] = "落凤一箭",
	
	["#sp_pangde"] = "枱榇之悟",
	["sp_pangde"] = "SP庞德",
	["&sp_pangde"] = "庞德",
	["illustrator:sp_pangde"] = "天空之城",
	["pangde_guiwei"] = "归魏",
	[":pangde_guiwei"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP庞德，势力为魏",
	["cv:sp_pangde"] = "Glory",

	["#sp_machao"] = "西凉的猛狮",
	["sp_machao"] = "SP马超",
	["&sp_machao"] = "马超",
	["illustrator:sp_machao"] = "天空之城",
	["fanqun"] = "返群",
	[":fanqun"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP马超，势力为群",

--hulao mode
	["Hulaopass"] = "虎牢关模式",

	["#shenlvbu1"] = "最强神话",
	["shenlvbu1"] = "虎牢关吕布",
	["&shenlvbu1"] = "最强神话",
	["illustrator:shenlvbu1"] = "LiuHeng",
	["#shenlvbu2"] = "暴怒的战神",
	["shenlvbu2"] = "虎牢关吕布",
	["&shenlvbu2"] = "暴怒战神",
	["illustrator:shenlvbu2"] = "LiuHeng",
	["xiuluo"] = "修罗",
	[":xiuluo"] = "回合开始阶段开始时，你可以弃置一张手牌，若如此做，你弃置你判定区里的一张与你弃置手牌同花色的延时类锦囊牌。",
	["@xiuluo"] = "请弃置一张花色相同的手牌",
	["shenwei"] = "神威",
	[":shenwei"] = "<b>锁定技</b>，摸牌阶段，你额外摸两张牌；你的手牌上限+2。",
	["shenji"] = "神戟",
	[":shenji"] = "若你的装备区没有武器牌，当你使用【杀】时，你可以额外选择至多两个目标。",

	["#Reforming"] = "%from 进入重整状态",
	["#ReformingRecover"] = "%from 在重整状态中回复了1点体力",
	["#ReformingDraw"] = "%from 在重整状态中摸了1张牌",
	["#ReformingRevive"] = "%from 从重整状态中复活!",
	["draw_1v3"] = "重整摸牌",
	["weapon_recast"] = "武器重铸",
	["Hulaopass:recover"] = "恢复1点体力",
	["Hulaopass:draw"] = "摸1张牌",
	
--sp_card
	["sp_cards"] = "SP卡牌包",
	["SPMoonSpear"] = "SP银月枪",
	[":SPMoonSpear"] = "装备牌·武器\
攻击范围：３\
武器特效：你的回合外，每当你使用或打出一张黑色手牌时，你可以令你攻击范围内的一名其他角色打出一张【闪】，否则受到你对其造成的1点伤害",
	["@moon-spear-jink"] = "受到SP银月枪技能的影响，你必须打出一张【闪】",]]
}
