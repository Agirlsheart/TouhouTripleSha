-- translation for Bangai Package

return {
	["touhou-bangai"] = "东方的白章",
	
	["#bangai001"] = "大空之弦",
	["bangai001"] = "宇佐见莲子",--风 - 空 - 4血
	["designer:bangai001"] = "幻桜落",
	["illustrator:bangai001"] = "安威拓郎",
	["cv:bangai001"] = "暂无",
	["thbianfang"] = "辨方", 
	[":thbianfang"] = "每当你使用【杀】对一名其他角色造成一次伤害后，或一名其他角色使用一张【杀】对你造成一次伤害后，你可以进行X次判定，每有一张判定牌花色与该【杀】相同，你便可以获得该角色的一张牌（X为你已损失的体力值，且至少为1）。",
	
	["#bangai002"] = "戾血的妖灵",
	["bangai002"] = "朱鹭子",--花 - 空 - 3血
	["designer:bangai002"] = "昂翼天使",
	["illustrator:bangai002"] = "DClockwork",
	["cv:bangai002"] = "暂无",
	["thshuling"] = "书灵",
	[":thshuling"] = "锁定技，游戏开始时，你获得1枚“妖书”标记。你或拥有该标记的其他角色的摸牌阶段摸牌时，额外摸一张牌，若你没有该标记，摸牌阶段结束时，你须将一张手牌交给拥有该标记的角色。你死亡时，将场上的该标记转移给一名其他角色，然后令其摸两张牌，并获得技能“书灵”。",
	["@thshuling"] = "你须令一名角色获得“妖书”标记，摸两张牌并获得“书灵”",
	["@thshuling-give"] = "请交给有标记的角色 %arg 张牌",
	["thshoujuan"] = "授卷",
	[":thshoujuan"] = "出牌阶段，你可将“妖书”标记转移给一名其他角色；拥有该标记的其他角色进入濒死状态时或对你造成一次伤害后，你可将你的人物牌翻面并获得其全部手牌，然后该角色须将该标记转移给你。",
	["@yaoshu"] = "妖书",
	
	["#bangai003"] = "胧望的幻灵",
	["bangai003"] = "大妖精",--雪 - 空 - 4血
	["designer:bangai003"] = "桃花僧",
	["illustrator:bangai003"] = "蓮芽しい",
	["cv:bangai003"] = "暂无",
	--thjibu
	["thzhiyue"] = "织月", 
	[":thzhiyue"] = "当你使用【杀】时，可以进行一次判定，若为黑色，额外指定一个目标；若为红色，此【杀】指定目标后，弃置目标角色一张牌。",
	["#ThZhiyue"]="%from 发动了“%arg”，为【%arg2】额外指定了一个目标 %to",
	
	["#bangai004"] = "星耀的使役魔",
	["bangai004"] = "小恶魔",--月 - 幻 - 3血
	["designer:bangai004"] = "桃花僧",
	["illustrator:bangai004"] = "konomi",
	["cv:bangai004"] = "暂无",
	["thzusha"] = "诅杀", 
	[":thzusha"] = "出牌阶段限一次，你可以弃置一张黑桃手牌并令一名其他角色获得1枚“诅咒”标记。其他角色的判定阶段开始时，若其拥有1枚或更多的“诅咒”标记，须进行一次判定，若为黑色，该角色失去1点体力；若为红色，弃置1枚“诅咒”标记。",
	["#ThZusha"] = "%from 受到了“%arg”的影响",
	["@zuzhou"] = "诅咒",
	["thyaomei"] = "妖魅", 
	[":thyaomei"] = "限定技，出牌阶段，你可以弃置一张红色手牌并选择两名已受伤的角色，已损失的体力值较小的角色失去1点体力（若相同，则由你选择一名角色），然后另一名角色回复1点体力。<br /><font color=\'red\'><b>操作提示：</font>你优先选择的目标为失去体力的角色。</b>",
	["thzhongjie"] = "忠节", 
	[":thzhongjie"] = "锁定技，若你的装备区里没有防具牌，你受到伤害时，若该伤害多于1点，则防止多余的伤害。",
	
	["#bangai005"] = "朝霭的幻世",
	["bangai005"] = "玛艾露贝莉•赫恩",--风 - 空 - 3血
	["&bangai005"] = "玛艾露贝莉",
	["designer:bangai005"] = "雷神之魂",
	["illustrator:bangai005"] = "十誤一会",
	["cv:bangai005"] = "暂无",
	["thxijing"] = "隙境", 
	[":thxijing"] = "你的回合外，每当你的非装备牌进入弃牌堆后，你可以用一张相同颜色的手牌替换之。",
	["@thxijing"] = "你可以用一张相同颜色的手牌替换 %src%dest 的 %arg",
	["thmengwei"] = "梦违", 
	[":thmengwei"] = "结束阶段开始时，若你的手牌小于两张，你可以将手牌补至两张；其他角色的准备阶段开始时，若你没有手牌，你可以摸一张牌。",
	
	["#bangai006"] = "幻梦的林檎",
	["bangai006"] = "艾丽",--花 - 空 - 4血
	["designer:bangai006"] = "幻桜落",
	["illustrator:bangai006"] = "UGUME",
	["cv:bangai006"] = "暂无",
	["thsilian"] = "死镰",
	[":thsilian"] = "锁定技，你手牌中的武器牌均视为【杀】；你获得即将进入你装备区的武器牌；若你的装备区没有武器牌，你视为装备着【离魂之镰】。",
	["thlingzhan"] = "灵战",
	[":thlingzhan"] = "每当你使用【杀】对目标角色造成一次伤害后，你可以进行一次判定，将非红桃的判定牌置于你的人物牌上称为“幻”，你可以将一张“幻”当【杀】使用或者打出。",
	["#thlingzhan"] = "灵战（操作判定牌）",
	["lingzhanpile"] = "幻",
	["thyanmeng"] = "衍梦",
	[":thyanmeng"] = "锁定技，其他角色不能令你的人物技能无效或失去。",
	
	["#bangai007"] = "荒城的苍影",
	["bangai007"] = "冴月麟",--雪 - 空 - 4血
	["designer:bangai007"] = "幻桜落",
	["illustrator:bangai007"] = "ForgottenGirlRin",
	["cv:bangai007"] = "暂无",
	["thqinshao"] = "琴韶",
	[":thqinshao"] = "弃牌阶段开始时，若你的手牌数大于体力值，你可以令一名其他角色摸X张牌；若你的手牌数小于体力值，你可以摸X张牌（X为你的手牌数与体力值之差）。",
	["@thqinshao"] = "你可以发动“琴韶”",
	["thxingxie"] = "星屑",
	[":thxingxie"] = "出牌阶段限一次，你可以弃置一张手牌，然后将一名角色装备区内的全部的牌置于你的人物牌上，此回合结束时，令该角色使用这些牌。",
	
	["#bangai008"] = "月都的玉兔",
	["bangai008"] = "泠仙",--月 - 空 - 3血
	["designer:bangai008"] = "幻桜落",
	["illustrator:bangai008"] = "tucana",
	["cv:bangai008"] = "暂无",
	["thyubo"] = "羽帛",
	[":thyubo"] = "出牌阶段限一次，你可以弃置一张黑色手牌并将至多两名角色的人物牌横置。",
	["thqiongfa"] = "穹法",
	[":thqiongfa"] = "人物牌横置的角色的结束阶段开始时，你可以令其选择一项：弃置由你指定的另一名角色的一张牌或令你摸一张牌。然后其重置其人物牌。",
	["#ThQiongfa"] = "%from 选择了 %to 作为被弃牌的目标",
	["@thqiongfa"] = "你可以弃置其他角色的一张牌，或点“取消”摸一张牌",
	["thqiongfa:discard"] = "弃置其指定的一名角色的一张牌",
	["thqiongfa:cancel"] = "令其摸一张牌",
	
	["#bangai009"] = "至高的龙神",
	["bangai009"] = "濑织津姬",--风 - 空 - 4血
	["designer:bangai009"] = "幻桜落",
	["illustrator:bangai009"] = "藤原",
	["cv:bangai009"] = "暂无",
	["thweide"] = "威德",
	[":thweide"] = "摸牌阶段摸牌时，你可以放弃摸牌，改为观看牌堆顶的两张牌并交给一名角色，然后若该角色不为你且你已受伤，你可以获得一名其他角色的一张手牌。",
	["@thweide"] = "你可以获得一名其他角色区域内的一张牌",
	
	["#bangai010"] = "禁书的评者",
	["bangai010"] = "本居小铃",--花 - 幻 - 3血
	["designer:bangai010"] = "xyzbilliu",
	["illustrator:bangai010"] = "湧々ごろり",
	["cv:bangai010"] = "暂无",
	["thguijuan"] = "诡卷",
	[":thguijuan"] = "出牌阶段，你可以摸一张牌，然后展示之并选择一项：使用此牌，或失去1点体力。若你以此法使用了一张【杀】或装备牌，你不可以发动“诡卷”，直到回合结束。",
	["@thguijuan"] = "请使用发动“诡卷”获得的牌，或点“取消”失去1点体力",
	["thzhayou"] = "诈诱",
	[":thzhayou"] = "当其他角色使用的【杀】被你的【闪】抵消后，你可以摸一张牌，然后令该角色选择一项：对你使用一张无视距离的【杀】，或受到你对其造成的1点伤害。",
	["@thzhayou"] = "%src 发动了“诈诱”，请对其使用一张无视距离的【杀】",
	
	["#bangai011"] = "神话的黄昏",
	["bangai011"] = "神绮",--雪 - 空 - 3血
	["designer:bangai011"] = "沸治·克里夫",
	["illustrator:bangai011"] = "湿布満",
	["cv:bangai011"] = "暂无",
	["thhuilun"] = "辉轮",
	[":thhuilun"] = "锁定技，你的黑色【杀】均视为【桃】；你的红色【桃】均视为【杀】。",
	["thwangdao"] = "妄道",
	[":thwangdao"] = "出牌阶段，你可以选择一名其他角色并展示一张【桃】，该角色须选择一项：对你使用一张花色相同的无视距离的【杀】；或获得此牌，然后令你选择弃置其两张牌或令其失去1点体力。",
	["@thwangdao"] = "受“妄道”影响，你需要对 %src 使用一张花色相同的无视距离的【杀】",
	["thwangdao:discard"] = "弃置其两张牌",
	["thwangdao:lose"] = "令其失去1点体力",
	
	["#bangai012"]="高天原之主",
	["bangai012"]="天照",--月 - 空 - 4血
	["designer:bangai012"]="昂翼天使",
	["illustrator:bangai012"]="水鼠",
	["cv:bangai012"]="暂无",
	["thsixiang"]="四象",
	[":thsixiang"]="出牌阶段，你可以选择一项：<br />1. 弃置两张黑桃牌并回复1点体力。<br />2. 弃置两张方块牌并弃置一名其他角色区域的一张牌。<br />3. 弃置两张梅花牌并令一名其他角色摸两张牌，然后该角色须弃置一张牌。<br />4. 弃置两张红桃牌并令一名其他角色摸一张牌，然后该角色将其人物牌翻面。",
}