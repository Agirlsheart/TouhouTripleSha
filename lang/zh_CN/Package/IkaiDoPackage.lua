-- translation for IkaiTsuchiPackage

-- translation for IkaiTsuchiPackage

return {
	["ikai-do"] = "异世界的土产",

--wind
	["#wind001"] = "终焉之理",
	["wind001"] = "鹿目圆香",--风 - 空 - 4血
	["designer:wind001"] = "游卡桌游",
	["illustrator:wind001"]="わたあめ",
	["ikshenai"] = "神爱",
	[":ikshenai"] = "出牌阶段限一次，你可以将至少一张手牌交给至少一名其他角色，若你以此法给出不少于两张牌，你回复1点体力。",
	["@ikshenai"] = "你可以发动“神爱”",
	["ikxinqi"] = "心契",
	[":ikxinqi"] = "君主技，当你需要使用或打出一张【杀】时，你可以令其他风势力角色打出一张【杀】（视为由你使用或打出）。",
	["@ikxinqi-slash"] = "请打出一张【杀】响应 %src “心契”",

	["#wind002"] = "赤发灼眼的猎手",
	["wind002"] = "夏娜",--风 - 空 - 4血
	["designer:wind002"] = "游卡桌游",
	["illustrator:wind002"]="舘津テト",
	["ikchilian"] = "赤莲",
	[":ikchilian"] = "你可以将一张红色牌当【杀】或具火焰伤害的【杀】使用或者打出。",
	["ikzhenhong"] = "真红",
	[":ikzhenhong"] = "锁定技，你使用红桃【杀】时无距离限制；你使用方块【杀】指定一名角色为目标后，无视其防具。",

	["#wind003"] = "小柄的空母",
	["wind003"] = "瑞凤",--风 - 空 - 4血
	["designer:wind003"] = "游卡桌游",
	["illustrator:wind003"]="双月アイジ",
	["ikyipao"] = "翼咆",
	[":ikyipao"] = "出牌阶段，你可以使用任意数量的【杀】。",
	["ikshijiu"] = "酾酒",
	[":ikshijiu"] = "你可以将一张武器牌或非延时类锦囊牌当【酒】使用。",

	["#wind004"] = "病弱的先知",
	["wind004"] = "园城寺怜",--风 - 空 - 3血
	["designer:wind004"] = "游卡桌游",
	["illustrator:wind004"]="フカヒレ",
	["ikyuxi"] = "预悉",
	[":ikyuxi"] = "准备阶段开始时，你可以观看牌堆顶的X张牌（X为存活角色的数量，且至多为5），将其中任意数量的牌以任意顺序置于牌堆顶，其余以任意顺序置于牌堆底。",
	["ikjingyou"] = "静幽",
	[":ikjingyou"] = "锁定技，若你没有手牌，你不能成为【杀】或者【碎月绮斗】的目标。",
	["#IkYuxiResult"] = "%from 的“<font color=\"yellow\"><b>预悉</b></font>”结果：%arg 上 %arg2 下",
	["$IkYuxiTop"] = "置于牌堆顶的牌：%card",
	["$IkYuxiBottom"] = "置于牌堆底的牌：%card",
	
	["#wind006"] = "亚瑟王",
	["wind006"] = "阿尔托莉雅•潘德拉根",--风 - 空 - 4血
	["&wind006"] = "阿尔托莉雅",
	--thjibu
	["designer:wind006"] = "游卡桌游",
	["illustrator:wind006"]="konomi",
	["ikyufeng"] = "御风",
	[":ikyufeng"] = "每当你使用【杀】指定一名角色为目标后，你可以令该角色的非锁定技无效，直到回合结束，然后你进行一次判定，令其选择一项：不能使用【闪】响应此【杀】；或弃置与判定结果花色相同的一张牌。",
	["@ikyufeng-discard"] = "请弃置一张 %arg 牌，否则你不能使用【闪】响应此【杀】",

	["#wind007"] = "金色妖精",
	["wind007"] = "维多利加•德•布洛瓦",--风 - 幻 - 3血
	["&wind007"] = "维多利加",
	["designer:wind007"] = "游卡桌游",
	["illustrator:wind007"]="華々つぼみ",
	["ikhuiquan"] = "慧泉",
	[":ikhuiquan"] = "当你使用一张锦囊牌时，你可以摸一张牌。",
	--thjizhi
	["ikhugu"] = "弧顾",
	[":ikhugu"] = "锁定技，其他角色不能弃置你装备区的武器牌或防具牌。",
	
	["#wind042"] = "华奢的夜兔",
	["wind042"] = "神乐",--风 - 空 - 4血
	["designer:wind042"] = "游卡桌游",
	["illustrator:wind042"]="hana's",
	["ikbaoou"] = "暴殴",
	[":ikbaoou"] = "其他角色的结束阶段开始时，若该角色于此回合内造成过伤害，你可以对其使用一张无视距离的【杀】。",
	["@ikbaoou-slash"] = "你可以发动“暴殴”对 %dest 使用一张【杀】",
	["ikyehua"] = "夜华",
	[":ikyehua"] = "觉醒技，当你造成伤害后，若你已受伤，你须减少1点体力上限，然后获得技能“星雨”（出牌阶段限一次，你可以选择一种牌的类别或颜色，然后亮出牌堆顶的一张牌，若此牌不为该类别或颜色，你重复此流程。你令一名角色获得最后亮出的牌，然后将其余的牌置入弃牌堆）。",
	["#IkYehuaWake"] = "%from 已受伤，触发“%arg”觉醒",
	["ikxingyu"] = "星雨",
	[":ikxingyu"] = "出牌阶段限一次，你可以选择一种牌的类别或颜色，然后亮出牌堆顶的一张牌，若此牌不为该类别或颜色，你重复此流程。你令一名角色获得最后亮出的牌，然后将其余的牌置入弃牌堆。",
	["#IkXingyuChoice"] = "%from 选择了 %arg",
	["@ikxingyu-give"] = "你可以令一名角色获得 %arg[%arg2]",
	
--bloom
	["#bloom001"] = "玉润的千金",
	["bloom001"] = "三千院凪",--花 - 空 - 4血
	["designer:bloom001"] = "游卡桌游",
	["illustrator:bloom001"]="くらすけ",
	["ikjiaoman"] = "娇蛮",
	[":ikjiaoman"] = "每当你受到一次伤害后，你可以将对你造成伤害的牌交给除伤害来源外的一名角色。",
	["@ikjiaoman"] = "你可以发动“娇蛮”",
	["ikhuanwei"] = "唤卫",
	[":ikhuanwei"] = "君主技，当你需要使用或打出一张【闪】时，你可以令其他花势力角色打出一张【闪】（视为由你使用或打出）。",
	["@ikhuanwei-jink"] = "请打出一张【闪】来保护 %src",

	["#bloom002"] = "壶中之天",
	["bloom002"] = "丹特丽安",--花 - 空 - 3血
	["designer:bloom002"] = "游卡桌游",
	["illustrator:bloom002"]="きゃら",
	["iktiansuo"] = "天锁",
	[":iktiansuo"] = "每当一名角色的判定牌生效前，你可以打出一张牌代替之。",
	["@iktiansuo-card"] = CommonTranslationTable["@askforretrial"],
	["~iktiansuo"] = "选择一张牌→点击确定",
	["ikhuanji"] = "幻姬",
	[":ikhuanji"] = "每当你受到1点伤害后，你可以获得伤害来源区域内的一张牌。",

	["#bloom003"] = "半月夜的呢喃",
	["bloom003"] = "秋庭里香",--花 - 空 - 4血
	["designer:bloom003"] = "游卡桌游",
	["illustrator:bloom003"]="山本ケイジ",
	["ikaoli"] = "傲戾",
	[":ikaoli"] = "每当你受到1点伤害后，可以进行一次判定，若结果为红色，你对伤害来源造成1点伤害；若结果为黑色，你弃置其区域内的一张牌。",
	["ikqingjian"] = "清俭",
	[":ikqingjian"] = "当你于摸牌阶段外获得牌时，你可以将其中至少一张牌交给至少一名其他角色，每回合限一次。",
	["@ikqingjian-distribute"] = "你可以发动“清俭”将 %arg 张牌任意分配给其他角色",

	["#bloom004"] = "龙凤的雀士",
	["bloom004"] = "松实宥＆松实玄",--花 - 空 - 4血
	["designer:bloom004"] = "游卡桌游",
	["illustrator:bloom004"]="NAbyssor",
	["ikchibao"] = "赤宝",
	[":ikchibao"] = "摸牌阶段，你可少摸至少一张牌，并选择手牌数不少于你的X名其他角色，获得这些角色的各一张手牌（X为你以此法少摸的牌的数量）。",
	["#ikchibao"] = "赤宝（后续结算）",
	["@ikchibao-card"] = "你可以发动“赤宝”选择至多 %arg 名其他角色",
	["~ikchibao"] = "选择若干名其他角色→点击确定",

	["#bloom005"] = "高岭之花",
	["bloom005"] = "葵喜美",--花 - 空 - 4血
	["designer:bloom005"] = "游卡桌游",
	["illustrator:bloom005"]="八永",
	["ikluoyi"] = "裸衣",
	[":ikluoyi"] = "你可以跳过摸牌阶段，并亮出牌堆顶的三张牌，然后获得其中的基本牌、武器牌和【碎月绮斗】，再将其余的牌置入弃牌堆，若如此做，你因执行【杀】或【碎月绮斗】的效果而造成的伤害+1，直到你的下回合开始。",
	["#IkLuoyiBuff"] = "%from 的“<font color=\"yellow\"><b>裸衣</b></font>”效果被触发，伤害从 %arg 点增加至 %arg2 点",

	["#bloom006"] = "夏日青空之影",
	["bloom006"] = "神尾观铃",--花 - 空 - 3血
	["designer:bloom006"] = "游卡桌游",
	["illustrator:bloom006"]="みわべさくら",
	["iktiandu"] = "天妒",
	[":iktiandu"] = "在你的判定牌生效后，你可以获得此牌。",
	["ikyumeng"] = "羽梦",
	[":ikyumeng"] = "每当你受到1点伤害后，可观看牌堆顶的两张牌，将其中一张交给一名角色，然后将另一张交给一名角色。",

	["#bloom007"] = "傲霜之蟹",
	["bloom007"] = "战场原黑仪",--花 - 幻 - 3血
	["designer:bloom007"] = "游卡桌游",
	["illustrator:bloom007"]="茶みらい",
	["ikmengyang"] = "濛漾",
	[":ikmengyang"] = "准备阶段开始时，你可以进行一次判定，若结果为黑色，你获得此牌，你可以重复此流程，直到出现红色的判定结果为止。",
	["#ikmengyang-move"]="濛漾（获得判定牌）",
	["ikzhongyan"] = "重岩",
	[":ikzhongyan"] = "你可以将一张黑色手牌当【闪】使用或打出。",

	["#bloom042"] = "链心的娇叶",
	["bloom042"] = "稻叶姬子",--花 - 空 - 3血
	["designer:bloom042"] = "游卡桌游",
	["illustrator:bloom042"]="nyanya",
	["ikxunxun"] = "恂恂",
	[":ikxunxun"] = "摸牌阶段开始时，你可以放弃摸牌，改为观看牌堆顶的四张牌并获得其中的两张牌，然后将其余的牌以任意顺序置于牌堆底。",
	["ikwangxi"] = "忘隙",
	[":ikwangxi"] = "每当你对其他角色造成1点伤害后，或受到其他角色造成的1点伤害后，你可以令你与该角色各摸一张牌。",

--snow
	["#snow001"] = "静夜的歌姬",
	["snow001"] = "拉克丝•克莱茵",--雪 - 空 - 4血
	["&snow001"] = "拉克丝",
	["designer:snow001"] = "幻桜落",
	["illustrator:snow001"]="KD",
	["ikzhiheng"] = "制衡",
	[":ikzhiheng"] = "出牌阶段限一次，你可以弃置一至X张牌，然后摸等量的牌（X为你的体力上限）。",
	["ikjiyuan"] = "济援",
	[":ikjiyuan"] = "君主技，锁定技，其他雪势力角色使用的【桃】指定你为目标后，回复的体力+1。",
	["#IkJiyuanExtraRecover"] = "%from 的“%arg”被触发，将额外回复 <font color=\"yellow\"><b>1</b></font> 点体力",

	["#snow002"] = "茧居神探",
	["snow002"] = "紫苑寺有子",--雪 - 空 - 4血
	["designer:snow002"] = "游卡桌游",
	["illustrator:snow002"]="岸田梅尔",
	["ikkuipo"] = "窥破",
	[":ikkuipo"] = "出牌阶段，你可以将一张黑色牌当【心网密葬】使用。",
	["ikguisi"] = "诡思",
	[":ikguisi"] = "限定技，当一名角色使用锦囊牌指定至少两名角色为目标时，你可以令此牌对其中的至少一名角色无效。",
	["@ikguisi-card"] = "你可以发动“诡思”",
	["~ikguisi"] = "选择任意名角色→点击确定",
	
	["#snow003"] = "棉被宇宙人",
	["snow003"] = "藤和艾莉欧",--雪 - 空 - 4血
	["ikbiju"] = "闭居",
	["designer:snow003"] = "游卡桌游",
	["illustrator:snow003"]="スカイ",
	[":ikbiju"] = "若你未于出牌阶段内使用或打出【杀】，你可以摸一张牌；或跳过此回合的弃牌阶段。",
	["ikpojian"] = "破茧",
	[":ikpojian"] = "觉醒技，结束阶段开始时，若你于此回合使用了不少于四张非装备牌，你须减少1点体力上限，并选择一项：回复1点体力或摸一张牌，然后获得技能“青华”（出牌阶段限一次，你可以令一名其他角色展示一张手牌，然后你可以弃置一张花色相同的牌，若如此做，该角色须弃置其展示的手牌，然后你可以与其各回复1点体力。）。",
	["ikpojian:recover"] = "回复1点体力",
	["ikpojian:draw"] = "摸一张牌",
	["#IkPojianWake"] = "%from 于此回合使用了 %arg 张非装备牌，触发“%arg2”觉醒",
	["ikqinghua"] = "青华",
	[":ikqinghua"] = "出牌阶段限一次，你可以令一名其他角色展示一张手牌，然后你可以弃置一张花色相同的牌，若如此做，该角色须弃置其展示的手牌，然后你可以与其各回复1点体力。",
	["@ikqinghua-discard"] = "你可以弃置一张 %arg 牌",

	["#snow004"] = "鬼畜的空间使",
	["snow004"] = "白井黑子",--雪 - 空 - 4血
	["designer:snow004"] = "游卡桌游",
	["illustrator:snow004"]="ペテン師",
	["ikkurou"] = "苦肉",
	[":ikkurou"] = "出牌阶段，你可以失去1点体力，然后摸两张牌。",
	["ikzaiqi"] = "再起",
	[":ikzaiqi"] = "当你于出牌阶段回复1点体力时，若你处于濒死状态，你可以摸一张牌。",

	["#snow005"] = "满帆的年华",
	["snow005"] = "冬海爱衣",--雪 - 空 - 3血
	["designer:snow005"] = "游卡桌游",
	["illustrator:snow005"]="yamasan",
	["ikguideng"] = "鬼灯",
	[":ikguideng"] = "出牌阶段，你可展示一张手牌并将之交给一名其他角色，令该角色选择一项：展示所有手牌并弃置与此牌花色相同的所有牌；或失去1点体力。",
	["ikguideng_discard:prompt"] = "你可以展示所有手牌并弃置所有 %arg 牌",
	["ikchenhong"] = "沉红",
	[":ikchenhong"] = "锁定技，摸牌阶段摸牌时，你须额外摸一张牌。你的手牌上限为X（X为你的体力上限）。",

	["#snow006"] = "矫矜的天狐",
	["snow006"] = "空幻",--雪 - 幻 - 3血
	["designer:snow006"] = "游卡桌游",
	["illustrator:snow006"]="じょな",
	["ikwanmei"] = "婉媚",
	[":ikwanmei"] = "出牌阶段限一次，你可选择一项：将一张方块牌当【春雪幻梦】使用；或弃置一张方块牌并弃置场上的一张【春雪幻梦】。若如此做，你摸一张牌。",
	["ikxuanhuo"] = "眩惑",
	[":ikxuanhuo"] = "当你成为【杀】的目标时，你可以弃置一张牌，然后将此【杀】转移给你攻击范围内的一名除【杀】的使用者外的角色。",
	["@ikxuanhuo"] = "%src 对你使用【杀】，你可以弃置一张牌发动“眩惑”",
	["~ikxuanhuo"] = "选择一张牌→选择一名其他角色→点击确定",

	["#snow007"] = "暗黑的圣杯",
	["snow007"] = "间桐樱",--雪 - 空 - 3血
	["designer:snow007"] = "游卡桌游",
	["illustrator:snow007"]="猫厨",
	["ikwujie"] = "无竭",
	[":ikwujie"] = "当你失去最后的手牌时，你可以摸一张牌。",
	["ikyuanhe"] = "渊壑",
	[":ikyuanhe"] = "出牌阶段限一次，你可以弃置一张红色手牌并指定一名其他角色，你与该角色各摸两张牌，然后各弃置两张牌。",

	["#snow008"] = "倔强的绿蔷薇",
	["snow008"] = "翠星石",--雪 - 幻 - 3血
	["designer:snow008"] = "游卡桌游",
	["illustrator:snow008"]="NAbyssor",
	["ikhuanlu"] = "浣露",
	[":ikhuanlu"] = "出牌阶段限一次，你可以弃置两张手牌并指定一名已受伤的其他空属性角色，你与其各回复1点体力。",
	["ikcangyou"] = "苍幽",
	[":ikcangyou"] = "当你失去一次装备区里的装备牌时，你可以摸两张牌。",

	["#snow042"] = "无口的女神",
	["snow042"] = "绫波丽",--雪 - 空 - 4血
	["designer:snow042"] = "游卡桌游",
	["illustrator:snow042"]="九十i",
	["ikziqiang"] = "自戕",
	[":ikziqiang"] = "出牌阶段限一次，你可以弃置一张牌，然后失去1点体力。",
	["iklingshili"] = "零式",
	[":iklingshili"] = "锁定技，每当你失去1点体力后，你摸三张牌，若此时在你的出牌阶段内，你能额外使用一张【杀】；你使用红色【杀】无距离限制，且目标角色不能使用【闪】响应之，直到回合结束。",
	["#iklingshili"] = "零式（强命）",

--luna
	["#luna002"] = "坚强的心之光",
	["luna002"] = "坂上智代",--月 - 空 - 4血
	["designer:luna002"] = "游卡桌游",
	["illustrator:luna002"]="かろりー",
	["ikwushuang"] = "无双",
	[":ikwushuang"] = "锁定技，当你使用【杀】指定一名角色为目标后，该角色需连续使用两张【闪】才能抵消；与你进行【碎月绮斗】的角色每次需连续打出两张【杀】。",
	["@ikwushuang-slash-1"] = "%src 对你【碎月绮斗】，你须连续打出两张【杀】",
	["@ikwushuang-slash-2"] = "%src 对你【碎月绮斗】，你须再打出一张【杀】",
	["ikwudi"] = "武帝",
	[":ikwudi"] = "出牌阶段限一次，你可以将两张相同花色的手牌当【碎月绮斗】使用。",

	["#luna003"] = "绝世的妖魂",
	["luna003"] = "玉藻前",--月 - 幻 - 3血
	["designer:luna003"] = "游卡桌游",
	["illustrator:luna003"]="tearfish",
	["ikqingguo"] = "倾国",
	[":ikqingguo"] = "出牌阶段限一次，你可以弃置一张牌并选择两名空属性角色，若如此做，视为其中一名空属性角色对另一名空属性角色使用一张【碎月绮斗】。此【碎月绮斗】不能被【三粒天滴】响应。",
	["ikbiyue"] = "闭月",
	[":ikbiyue"] = "回合结束阶段开始时，你可以摸一张牌。",

	["#luna006"] = "森之轮回",
	["luna006"] = "艾露露",--月 - 空 - 3血
	["designer:luna006"] = "游卡桌游",
	["illustrator:luna006"]="天夢森流彩",
	["ikhuichun"] = "回春",
	[":ikhuichun"] = "你的回合外，你可以将一张红色牌当【桃】使用。",
	["ikqingnang"] = "青囊",
	[":ikqingnang"] = "出牌阶段限一次，你可以弃置一张手牌，令一名已受伤的角色回复1点体力。",

	["#luna018"] = "默然的空之音",
	["luna018"] = "筒隐月子",--月 - 空 - 3血
	["designer:luna018"] = "KayaK",
	["illustrator:luna018"]="カントク",
	["ikzhuji"] = "祝祭",
	[":ikzhuji"] = "锁定技，若你的体力值大于2，当你计算与其他角色的距离时，始终-1；若你的体力值不大于2，当其他角色计算与你的距离时，始终+1。",
	["ikbenyin"] = "本音",
	[":ikbenyin"] = "每当你使用黑色【杀】对一名角色造成伤害后，你可以选择一项：弃置该角色装备区里的一张非坐骑牌；或获得该角色装备区里的一张坐骑牌。",
	["ikguizhi"] = "鬼贽",
	[":ikguizhi"] = "每当一名其他角色进入濒死状态时，若你的体力值大于1，你可以失去1点体力并弃置一张牌，然后令其回复1点体力。",
	["@ikguizhi"] = "你可以发动“鬼贽”",

	["#luna034"] = "禁断之缘",
	["luna034"] = "春日野穹",--月 - 空 - 4血
	["designer:luna034"] = "游卡桌游",
	["illustrator:luna034"]="ちり",
	["ikguijiao"] = "鬼娇",
	[":ikguijiao"] = "准备阶段开始时，若所有角色都没有拥有“恶娇”标记，你可以指定一名其他角色，令其获得一枚“恶娇”标记。拥有“恶娇”标记的角色的准备阶段开始时，你可以摸一张牌，若如此做，此回合该角色的手牌上限-1。",
	["@ikguijiao"] = "你可以发动“鬼娇”",
	["@ejiao"] = "恶娇",
	["ikjinlian"] = "禁恋",
	[":ikjinlian"] = "锁定技，若你的手牌数大于体力值，你在其攻击范围内的其他角色使用【杀】时，不能指定除你以外的角色为目标。",
	
	["#luna042"] = "童话的魔女",
	["luna042"] = "久远寺有珠",--月 - 空 - 3血
	--ikhuichun
	["designer:luna042"] = "游卡桌游",
	["illustrator:luna042"]="こやまひろかず",
	["ikyaoge"] = "药割",
	[":ikyaoge"] = "出牌阶段，若你有牌，你可以选择有牌的势力各不同的至少一名其他角色，弃置你和这些角色的各一张牌，然后以此法被弃置黑桃牌的角色各摸一张牌。",

}