-- translation for WindPackage

return {
	["wind"] = "风包",

	["#caoren"] = "大将军",
	["caoren"] = "曹仁",
	["jushou"] = "据守",
	[":jushou"] = "回合结束阶段开始时，你可以摸三张牌，然后将你的武将牌翻面。",
	["jushou:yes"] = "摸三张牌，然后将你的武将翻面",

	["#huangzhong"] = "老当益壮",
	["huangzhong"] = "黄忠",
	["liegong"] = "烈弓",
	[":liegong"] = "当你在出牌阶段内使用【杀】指定一名角色为目标后，以下两种情况，你可以令其不可以使用【闪】对此【杀】进行响应：1.目标角色的手牌数大于或等于你的体力值。2.目标角色的手牌数小于或等于你的攻击范围。",
	["liegong:yes"] = "此【杀】不可被【闪】响应",

	["#weiyan"] = "嗜血的独狼",
	["weiyan"] = "魏延",
	["illustrator:weiyan"] = "Sonia Tang",
	["kuanggu"] = "狂骨",
	[":kuanggu"] = "<b>锁定技</b>，每当你对距离1以内的一名角色造成1点伤害后，你回复1点体力。 ",

	["#xiaoqiao"] = "矫情之花",
	["xiaoqiao"] = "小乔",
    ["tianxiang"] = "天香",
	[":tianxiang"] = "每当你受到伤害时，你可以弃置一张红桃手牌，将此伤害转移给一名其他角色，然后该角色摸X张牌（X为该角色当前已损失的体力值）。\
◆你受到一张牌对你造成的伤害时发动【天香】转移的是伤害而不是牌，因此不需要进行使用结算，直接进行伤害结算。",
	["@tianxiang-card"] = "请弃置一张红桃手牌来转移伤害",
	["~tianxiang"] = "选择一张红桃手牌→选择一名其他角色→点击确定",
	["hongyan"] = "红颜",
	[":hongyan"] = "<b>锁定技</b>，你的黑桃牌均视为红桃牌。",
	
	["#zhoutai"] = "历战之驱",
	["zhoutai"] = "周泰",
	["buqu"] = "不屈",
	[":buqu"] = "每当你扣减1点体力后，若你当前的体力值为0：你可以从牌堆顶亮出一张牌置于你的武将牌上，若此牌的点数与你武将牌上已有的任何一张牌都不同，你不会死亡；若出现相同点数的牌，你进入濒死状态。\
◆置于你的武将牌上的牌是移出游戏的牌。",
	["#BuquDuplicate"] = "%from 时运不济啊，不屈牌中有 %arg 重复点数",
	["#BuquDuplicateGroup"] = "第 %arg 组重复点数为 %arg2",
	["$BuquDuplicateItem"] = "不屈重复牌: %card",
	["$BuquRemove"] = "%from 移除了“不屈牌”：%card", 

	["#yuji"] = "太平道人",
	--["yuji"] = "于吉",
	["illustrator:yuji"] = "LiuHeng",
	["guhuo"] = "蛊惑",
	[":guhuo"] = "你可以说出一张基本牌或非延时类锦囊牌的名称，并背面朝上使用或打出一张手牌。若无其他角色质疑，则亮出此牌并按你所述之牌结算。若有其他角色质疑则亮出验明：若为真，质疑者各失去1点体力；若为假，质疑者各摸一张牌。除非被质疑的牌为红桃且为真，此牌仍然进行结算，否则无论真假，将此牌置入弃牌堆。\
◆当前的体力值为0的角色不能质疑。",
	["guhuo_pile"] = "蛊惑牌堆",
	["#Guhuo"] = "%from 发动【%arg2】，据说这张牌是 %arg，选择的目标为 %to",
	["#GuhuoNoTarget"] = "%from 发动【%arg2】，据说这张牌是 %arg",
	["#GuhuoCannotQuestion"] = "%from 体力不支(当前体力值=%arg)，无法质疑",
	["guhuo:question"] = "质疑",
	["guhuo:noquestion"] = "不质疑",
	["question"] = "质疑",
	["noquestion"] = "不质疑",
	["#GuhuoQuery"] = "%from 表示 %arg",
	["$GuhuoResult"] = "%from 用来蛊惑的牌是 %card",
	["guhuo-saveself"] = "自救蛊惑",
	["guhuo-saveself:peach"] = "桃子",
	["guhuo-saveself:analeptic"] = "酒",

-- guhuo dialog
	["single_target"] = "单目标锦囊",
	["multiple_targets"] = "多目标锦囊",
}
