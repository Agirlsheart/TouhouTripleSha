-- translation for Jianniang Scenario

return {
	["jianniang"] = "拂晓之航迹",

--大和
	["jndaizhan"] = "怠战",
	[":jndaizhan"] = "锁定技，你的技能\"<b>战歌</b>\"无效，且你不能使用【杀】或成为【杀】的目标，直到你发动\"<b>超弩</b>\"。",

	["jnchaonu"] = "超弩",
	[":jnchaonu"] = "限定技，一名角色的回合结束后，若你已受伤，你可以进行一个额外的回合。此额外回合的出牌阶段，你可以额外使用一张【杀】。",

	["jnqishui"] = "期水",
	[":jnqishui"] = "觉醒技，准备阶段开始时，若你的体力值为1，你弃置装备区的武器牌并获得技能\"<b>神薙</b>\"（每当你使用【杀】时，若你的装备区没有武器牌，你可以额外指定至多两个目标。）。若你的技能\"<b>战歌</b>\"已发动，则视为未发动。",
	["#JnQishui"] = "%from 的体力值为 %arg2，触发“%arg”觉醒",
	
	["ikshenti"] = "神薙",
	[":ikshenti"] = "每当你使用【杀】时，若你的装备区没有武器牌，你可以额外指定至多两个目标。",

--榛名
	["jnxiangwu"] = "祥武",
	[":jnxiangwu"] = "限定技，当你进入濒死状态时，你可以将全部的手牌交给一名其他角色。若如此做，该角色可以对你使用一张【桃】，若该角色如此做，你获得技能\"<b>礼惠</b>\"，且若该角色为异端，你的身份牌改为异端；否则你与该角色的身份牌改为司祝。",
	["@jnxiangwu"] = "你可以选择一名其他角色发动“祥武”",

	["jnkongwu"] = "空吴",
	[":jnkongwu"] = "觉醒技，当你脱离濒死状态时，获得技能\"<b>空睨</b>\"（锁定技，你的装备区的坐骑牌无效，你无视其他角色装备区的防具牌与坐骑牌，你使用的【杀】指定一名体力值大于你的其他角色为目标后，此【杀】不可被【闪】响应。）。",
	
	["ikkongni"] = "空睨",
	[":ikkongni"] = "锁定技，你的装备区的坐骑牌无效，你无视其他角色装备区的防具牌与坐骑牌，你使用的【杀】指定一名体力值大于你的其他角色为目标后，此【杀】不可被【闪】响应。",

--响
	["jnhuangqi"] = "凰启",
	[":jnhuangqi"] = "觉醒技，准备阶段开始时，若你已发动\"<b>碎涅</b>\"，你摸一张牌并获得技能\"<b>殁槃</b>\"（当你处于濒死状态时，你可以将一张红色手牌当【桃】使用。）。",

	["ikmopan"] = "殁槃",
	[":ikmopan"] = "当你处于濒死状态时，你可以将一张红色手牌当【桃】使用。",

	["jnlaishi"] = "赖实",
	[":jnlaishi"] = "限定技，当你于濒死状态回复体力后，若体力值不为0且回复来源为其他角色，你可以令其摸一张牌，然后若该角色为司祝，你的身份牌改为司祝；否则你与该角色的身份牌改为异端。",

--夕立
	["jnluomo"] = "罗魔",
	[":jnluomo"] = "觉醒技，准备阶段开始时，若你的体力值为1，你回复1点体力或摸两张牌，然后获得技能\"<b>叹魇</b>\"（每当你需要使用或打出一张【杀】时，你可以展示一张黑色手牌。若如此做，视为你使用或打出了一张【杀】，每回合限一次。）。",
	["jnluomo:recover"] = "回复1点体力",
	["jnluomo:draw"] = "摸两张牌",

	["iktanyan"] = "叹魇",
	[":iktanyan"] = "每当你需要使用或打出一张【杀】时，你可以展示一张黑色手牌。若如此做，视为你使用或打出了一张【杀】，每回合限一次。",

--瑞凤
	["jnqiwang"] = "崎往",
	[":jnqiwang"] = "锁定技，游戏开始时，你获得技能\"<b>青囊</b>\"，且技能\"<b>翼咆</b>\"和\"<b>酾酒</b>\"无效，直到你发动\"<b>临兵</b>\"。",

	["jnlinbing"] = "临兵",
	[":jnlinbing"] = "觉醒技，当你受到一次伤害后，若你已损失的体力值不小于2，你将体力补至等同于你体力上限，然后获得技能\"<b>遐闪</b>\"（你于出牌阶段使用的【杀】可以无视合法性指定一名角色为目标。若目标角色不在你的攻击范围内，则\"<b>翼咆</b>\"无效，直到此回合结束。）。",

	["ikxiashan"] = "遐闪",
	[":ikxiashan"] = "你于出牌阶段使用的【杀】可以无视合法性指定一名角色为目标。若目标角色不在你的攻击范围内，则\"<b>翼咆</b>\"无效，直到此回合结束。",
	["@ikxiashan"] = "你可以无视合法性指定一名角色为目标",

--加贺
	["jnbizheng"] = "必正",
	[":jnbizheng"] = "限定技，当一名其他角色受到伤害时，你可以将此伤害转移给自己。",

	["jnmingshi"] = "命矢",
	[":jnmingshi"] = "限定技，出牌阶段开始时，你可以指定至多等同于手牌数的其他角色，然后你依次无视合法性将一张手牌当做【枯羽华庭】置于这些角色的判定区内，然后弃置全部的手牌。若如此做，视为你依次对这些角色使用了一张无视距离的【杀】。",
	["@jnmingshi"] = "你可以发动“命矢”",
	["~jnmingshi"] = "选择至多等同于手牌数的其他角色→点击确定",
	["@jnmingshi-card"] = "请选择一张手牌当做【枯羽华庭】置于 %src 的判定区内",
	
--北上
	["jnyiwu"] = "亦无",
	[":jnyiwu"] = "锁定技，你的出牌阶段至多使用X张牌（X为其此出牌阶段开始时的体力值），直到你发动\"<b>重雷</b>\"。",

	["jnzhonglei"] = "重雷",
	[":jnzhonglei"] = "觉醒技，准备阶段开始时，若你的体力值为全场最低（或之一），你获得技能\"<b>巡律</b>\"（你可以将一张黑色牌当不计入每阶段次数限制的【杀】使用或打出。）。",

	["ikxunlv"] = "巡律",
	[":ikxunlv"] = "你可以将一张黑色牌当不计入每阶段次数限制的【杀】使用或打出。",

--岛风
	["jnjicha"] = "疾刹",
	[":jnjicha"] = "锁定技，若你的体力值大于1，你的红色【净琉璃镜】的判定结果视为红色；若你的体力值为1，其他体力值不为1的角色计算与你的距离时至少为2。",
	
--爱宕
	["jnyuhe"] = "玉和",
	[":jnyuhe"] = "出牌阶段开始时，你可以弃置一张手牌，若如此做，你于此阶段每发动一次\"<b>零刹</b>\"，可以令一名角色摸一张牌。",
	["@jnyuhe"] = "你可以弃置一张手牌发动“玉和”",
	["@jnyuhe-draw"] = "你可以令一名角色摸一张牌",

	["jnyanwang"] = "烟惘",
	[":jnyanwang"] = "觉醒技，你发动\"<b>零刹</b>\"，在结算后，若存活角色数小于你的当前体力值，你须减少1点体力上限。然后你的技能\"<b>零刹</b>\"改为出牌阶段限两次，且可以以此法当【心网密葬】使用。",
	
--弥生
	["jnxianmao"] = "贤卯",
	[":jnxianmao"] = "限定技，一名其他角色的回合结束后，若其在该回合对你造成过伤害，你可以进行一个额外的回合，在此额外的回合内，你使用的牌须无视合理性指定该角色为目标，且此回合的结束阶段开始时，你将人物牌翻面。",
	["@jnxianmao"] = "收“贤卯”影响，你于本回合使用的牌须无视合理性指定该角色为目标",

	["jnchunyu"] = "椿雨",
	[":jnchunyu"] = "限定技，一名其他角色死亡后，你可以获得其区域内所有的牌，并获得技能\"<b>疾智</b>\"。",

	["jnsongyi"] = "松遗",
	[":jnsongyi"] = "觉醒技，准备阶段开始时，若你的体力值为1，你获得技能\"<b>赤莲</b>\"。",
	
--赤城
	["jntaozui"] = "饕罪",
	[":jntaozui"] = "锁定技，游戏开始时，你增加2点体力上限，然后回复2点体力。摸牌阶段，你额外摸两张牌。你无法使用或打出除【杀】外的基本牌，直到\"<b>零烈</b>\"发生。",

	["jnlinglie"] = "零烈",
	[":jnlinglie"] = "觉醒技，准备阶段开始时，若你的体力值不大于3，须减少3点体力上限，体力回复至3点，你的技能\"<b>苍流</b>\"中失去效果时机由\"<b>手牌数变化时</b>\"改为\"<b>手牌数增加时</b>\"。",

--曙
	["jndongao"] = "冬傲",
	[":jndongao"] = "每当你受到伤害时，若此伤害大于1，你可以弃置一张牌，防止多余的伤害。",
	["@jndongao"] = "你可以弃置一张牌发动“冬傲”",

	["jnchunsu"] = "春诉",
	[":jnchunsu"] = "限定技，准备阶段开始时，你可以选择一名其他角色，交给其X张手牌，然后将其势力属性改变为风势力（X为你的体力值）。若如此做，防止你对其造成的伤害，直到回合结束。",
	["@jnchunsu"] = "你可以发动“春诉”",
	["~jnchunsu"] = "选择X张手牌→选择一名其他角色→点击确定",
	["#jnchunsu"] = "春诉（防止伤害）",

	["jnxiamu"] = "夏睦",
	[":jnxiamu"] = "限定技，当你进入濒死状态时，你可以回复2点体力，并摸X张牌（X存活角色数且至多为4）。",

	["jnqiuling"] = "秋铃",
	[":jnqiuling"] = "锁定技，你死亡时，杀死你的角色弃置一张手牌，然后除该角色外的所有角色摸一张牌。",

--伊十九
	["jnangong"] = "暗供",
	[":jnangong"] = "出牌阶段限一次，若你人物牌上的牌小于三张，你可以将一张基本牌置于人物牌上。",
	["jnangongpile"] = "暗供",

	["jnhuoji"] = "火给",
	[":jnhuoji"] = "每当你需要使用或打出一张基本牌时，你可以将人物牌上的一张牌置入弃牌堆，视为使用或打出了一张该基本牌，然后摸一张牌。",
	["jnhuoji_slash"] = "火给出杀",
	["jnhuoji_saveself"] = "火给自救",
	["#jnhuoji"] = "火给（摸牌）",

	["jnniying"] = "匿影",
	[":jnniying"] = "锁定技，游戏开始时，你获得1枚“潜”标记。拥有该标记时，其他角色对你使用、你对其他角色使用除【百鬼夜行】、【魔闪花火】、【神惠雨降】、【竹取谜宝】或【荒波暴流】外的牌均无效。结束阶段开始时，你须弃置全部该标记。",
	["@qianhang"] = "潜",

	["jntaoxi"] = "涛袭",
	[":jntaoxi"] = "回合结束时，你可以弃置你人物牌上的等同于你体力值的牌，并获得1枚“潜”标记。一名角色的出牌阶段开始时，你可以弃置全部的该标记，然后若当前回合角色手牌数大于你，你可以对其发动“<b>潜慑</b>”，若小于你，你可以对其发动“<b>导雷</b>”。",
	["@jntaoxi"] = "你可以发动“涛袭”",
	["~jntaoxi"] = "选择等同于你体力值的牌→点击确定",
}
