-- translation for MountainPackage

return {
	["mountain"] = "山包",

	["#dengai"] = "矫然的壮士",
	["dengai"] = "邓艾",
	["tuntian"] = "屯田",
	[":tuntian"] = "你的回合外，当你失去牌时，你可以进行一次判定，将非红桃结果的判定牌置于你的武将牌上，称为“田”；每有一张“田”，你计算的与其他角色的距离便-1。\
◆田是移出游戏的牌。",
	["jixi"] = "急袭",
	[":jixi"] = "你可以将一张“田”当【顺手牵羊】使用。",
	["zaoxian"] = "凿险",
	[":zaoxian"] = "<b>觉醒技</b>，回合开始阶段开始时，若“田”的数量达到3或更多，你须减1点体力上限，并获得技能“急袭”（你可以将一张“田”当【顺手牵羊】使用）。",
	["#ZaoxianWake"] = "%from 的田的数量达到 %arg 个，触发【%arg2】",
	["field"] = "田",

	["#liushan"] = "无为的真命主",
	["liushan"] = "刘禅",
	["illustrator:liushan"] = "LiuHeng",
	["xiangle"] = "享乐",
	[":xiangle"] = "<b>锁定技</b>，当其他角色使用【杀】指定你为目标时，需弃置一张基本牌，否则此【杀】对你无效。",
	["fangquan"] = "放权",
	[":fangquan"] = "你可以跳过你的出牌阶段，若如此做，你在回合结束时可以弃置一张手牌令一名其他角色进行一个额外的回合。",
	["ruoyu"] = "若愚",
	[":ruoyu"] = "<b>主公技</b>，<b>觉醒技</b>，回合开始阶段开始时，若你的体力是全场最少的（或之一），你须加1点体力上限，回复1点体力，并获得技能“激将”。",
	["#Xiangle"] = "%from 受到【%arg】的影响，需弃置一张基本牌才能令此【杀】对%to 生效。",
	["#Fangquan"] = "%from 发动【放权】，%to 将进行一个额外的回合。",
	["#RuoyuWake"] = "%from 的体力值(%arg)为全场最少，触发【%arg2】。",
	["@xiangle-discard"] = "你需弃置一张基本牌才能令此【杀】生效",

	["#jiangwei"] = "龙的衣钵",
	["jiangwei"] = "姜维",
	["tiaoxin"] = "挑衅",
	[":tiaoxin"] = "出牌阶段，你可以令一名你在其攻击范围内的其他角色选择一项：对你使用一张【杀】，或令你弃置其一张牌。每阶段限一次。",
	["zhiji"] = "志继",
	[":zhiji"] = "<b>觉醒技</b>，回合开始阶段开始时，若你没有手牌，你须选择一项：回复1点体力，或摸两张牌。然后你减1点体力上限，并获得技能“观星”。",
	["zhiji:draw"] = "摸两张牌",
	["zhiji:recover"] = "回复1点体力",
	["#ZhijiWake"] = "%from 没有手牌，触发【%arg】",
	["@tiaoxin-slash"] = "%src 向你发动【挑衅】，请对其使用一张【杀】",

	["#sunce"] = "江东的小霸王",
	["sunce"] = "孙策",
	["jiang"] = "激昂",
	[":jiang"] = "每当你使用（指定目标后）或被使用（成为目标后）一张【决斗】或红色的【杀】时，你可以摸一张牌。",
	["hunzi"] = "魂姿",
	[":hunzi"] = "<b>觉醒技</b>，回合开始阶段开始时，若你的体力为1，你须减1点体力上限，并获得技能“英姿”和“英魂”。",
	["sunce_zhiba"] = "制霸",
	["zhiba_pindian"] = "制霸拼点",
	[":sunce_zhiba"] = "<b>主公技</b>，其他吴势力角色可以在他们各自的出牌阶段与你拼点（“魂姿”发动后，你可以拒绝此拼点），若该角色没赢，你可以获得双方拼点的牌。每阶段限一次。\
◆吴势力角色发动【制霸】与你拼点被拒绝计入出牌阶段的发动次数限制。",
	["#HunziWake"] = "%from 的体力为 1，触发【%arg】",
	["zhiba_pindian:accept"] = "接受",
	["zhiba_pindian:reject"] = "拒绝",
	["sunce_zhiba:yes"] = "获得拼点牌",
	["sunce_zhiba:no"] = "不获得",

	["#erzhang"] = "经天纬地",
	["erzhang"] = "张昭张纮",
	["illustrator:erzhang"] = "废柴男",
	["zhijian"] = "直谏",
	[":zhijian"] = "出牌阶段，你可以将手牌中的一张装备牌置于一名其他角色的装备区里（不能替换原装备），然后摸一张牌。\
◆你发动【直谏】即对目标角色使用了一张装备牌。",
	["guzheng"] = "固政",
	[":guzheng"] = "其他角色的弃牌阶段结束时，你可以将该角色于此阶段中弃置的一张牌从弃牌堆返回其手牌，若如此做，你可以获得弃牌堆里其余于此阶段中弃置的牌。\
◆你发动【固政】必须将该角色于此阶段中自己弃置自己的手牌中的一张返回该角色的手牌，然后你可以获得弃牌堆里其余于此阶段中任何角色因任何原因弃置的牌。",
	["$ZhijianEquip"] = "%from 被装备了 %card",

	["#zuoci"] = "迷之仙人",
	["zuoci"] = "左慈",
	["illustrator:zuoci"] = "废柴男",
	["#zuocif"] = "迷之仙女",
	["zuocif"] = "左慈(女)",
	["illustrator:zuocif"] = "天宫十月",
	["huashen"] = "化身",
	[":huashen"] ="所有人都展示武将牌后，你随机获得两张未加入游戏的武将牌，称为“化身牌”，选一张置于你面前并声明该武将的一项技能，你获得该技能且同时将性别和势力属性变成与该武将相同直到“化身牌”被替换。在你的每个回合开始时和结束后，你可以替换“化身牌”，然后（无论是否替换）你为当前的“化身牌”声明一项技能（你不可以声明限定技、觉醒技或主公技）。\
◆你的“化身牌”背面朝上置于一旁，形成“化身库”，你声明技能后将“化身牌”正面朝上置于你的武将牌旁，被替换后仍背面朝上放回“化身库”，每张“化身牌”的武将技能都可以被反复声明。\
◆如果你当前拥有的技能将游戏牌移出游戏或能获得标记，你发动【化身②】更换技能时将被你移出游戏的牌全部置入弃牌堆或弃全部标记。\
◆你发动【化身②】更换技能是在失去原技能的同时拥有新技能。\
◆你不可以声明由于触发觉醒技而获得的技能。",
	["xinsheng"] = "新生",
	[":xinsheng"] = "每当你受到1点伤害后，你可以获得一张“化身牌”。\
◆你获得新化身牌是从未加入游戏的武将牌堆中随机抽取且不展示，直接背面朝上置入化身库。\
◆死亡角色的武将牌加入未加入游戏的武将牌堆中。",
	["#GetHuashen"] = "%from 获得了 %arg 张化身牌，现在共有 %arg2 个化身",

-- CV
	["cv:zhanghe"] = "爪子",
	["$mancai1"] = "虚招令旗，以之惑敌。", -- judge
	["$mancai2"] = "绝其汲道，困其刍粮。", -- draw
	["$mancai3"] = "以守为攻，后发制人。", -- play
	["$mancai4"] = "停止前进，扎营御敌！", -- discard
	["~zhanghe"] = "归兵勿追，追兵难归啊……",

	["cv:dengai"] = "烨子",
	["$tuntian1"] = "食者、兵之所系；农者、胜之所依。",
	["$tuntian2"] = "积军资之粮，通漕运之道。",
	["$zaoxian"] = "孤注一掷，胜败在此一举！",
	["$jixi1"] = "给我一张，又何妨？",
	["$jixi2"] = "明修栈道，暗度陈仓。",
	["~dengai"] = "吾破蜀克敌，竟葬于奸贼之手！",

	["cv:liushan"] = "V7",
	["$fangquan1"] = "一切但凭相父做主",
	["$fangquan2"] = "孩儿愚钝",
	["$ruoyu"] = "大智若愚，大巧若拙",
	["$xiangle1"] = "此间乐，不思蜀",
	["$xiangle2"] = "如此甚好，甚好",
	["~liushan"] = "吾降，吾降矣",

	["cv:jiangwei"] = "Jr. Wakaran",
	["$tiaoxin1"] = "贼将早降，可免一死！",
	["$tiaoxin2"] = "哼，贼将莫不是怕了？",
	["$tiaoxin3"] = "敌将可破得我八阵？", -- 装备了八卦或有八阵技能时发动
	["$zhiji"] = "今虽穷极，然先帝之志，丞相之托，维岂敢忘！",
	["$Zhiji"] = "今虽穷极\
然先帝之志、丞相之托\
维岂敢忘！",
	["~jiangwei"] = "臣等正欲死战，陛下何故先降？",

	["cv:sunce"] = "猎狐",
	["$jiang1"] = "所向皆破，敌莫敢当。",
	["$jiang2"] = "众将听令，直讨敌酋。",
	["$hunzi"] = "父亲在上，魂佑江东；公瑾在旁，智定天下！",
	["$Hunzi"] = "父亲在上　魂佑江东\
公瑾在旁　智定天下",
	["$sunce_zhiba1"] = "子义信义，必不背我。", --同意制霸
	["$sunce_zhiba2"] = "哈哈，汝乃吾之真卿也！", --主公得利
	["$sunce_zhiba3"] = "哼，错当佞臣做忠臣，誓不饶你！", --反贼得利
	["$sunce_zhiba4"] = "且慢！莫不是你欲用诈降之计赚我。", --拒绝制霸
	["~sunce"] = "内事不决问张昭，外事不决问周瑜……",

	["cv:erzhang"] = "",
	["$zhijian"] = "忠謇方直，动不为己。",
	["$guzheng1"] = "隐息师徒，广开播殖。",
	["$guzheng2"] = "任贤使能，务崇宽惠。",
	["~erzhang"] = "此生智谋，已为江东尽……",

	["cv:caiwenji"] = "呼呼",
	["$huiyao1"] = "欲死不能得，欲生无一可。", -- club
	["$huiyao2"] = "此行远兮，君尚珍重！", -- spade
	["$huiyao3"] = "翩翩吹我衣，肃肃入我耳。", -- diamond
	["$huiyao4"] = "岂偕老之可期，庶尽欢于余年。", -- heart
	["$qihuang1"] = "雁飞高兮邈难寻，空断肠兮思愔愔。",
	["$qihuang2"] = "胡人落泪沾边草，汉使断肠对归客。",

	["cv:zuoci"] = "东方胤弘",
	["$huashen1"] = "藏形变身,自在吾心(男声)",
	["$huashen2"] = "遁形幻千,随意所欲(男声)",
	["$xinsheng1"] = "吐故纳新,师法天地(男声)",
	["$xinsheng2"] = "灵根不灭,连绵不绝(男声)",
	["~zuoci"] = "释知遗形,神灭形消",

	["cv:zuocif"] = "眠眠",
	["$huashen3"] = "藏形变身,自在吾心(女声)",
	["$huashen4"] = "遁形幻千,随意所欲(女声)",
	["$xinsheng3"] = "吐故纳新,师法天地(女声)",
	["$xinsheng4"] = "灵根不灭,连绵不绝(女声)",
}
