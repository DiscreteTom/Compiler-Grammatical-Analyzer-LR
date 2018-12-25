# 语法分析器的设计与实现-LR

## 要求（摘自课本

**实验内容** - 编写语法分析程序，实现对算术表达式的语法分析。要求所分析算术表达式由如下文法产生：

```
E -> E+T | E-T | T
T -> T*F | T/F | F
F -> (E) | num
```

**实验要求**
- 在对输入的算术表达式进行分析的过程中，依次输出所采用的产生式。
- 构造识别该文法所有活前缀的DFA
- 构造文法的LR分析表
- 编程实现算法4.3，构造LR分析程序

## 算法4.3（摘自课本

- 输入 - 文法G的一张分析表和一个输入符号串ω
- 输出 - 如果ω∈L(G)，得到ω自底向上的分析，否则报错

方法：

```c++
初始化 {
	初始状态S0压栈
	ω$存入输入缓冲区
	置ip指向ω$的第一个符号
}
do {
	令S是栈顶状态，a是ip指向的符号;
	if (action[S, a] = shift S'){
		把a和S'分别压入符号栈和状态栈的栈顶;
		推进ip，使它进入下一个输入符号;
	} else if (action[S, a] = reduce by A -> β){
		从栈顶弹出|β|个符号;
		令S'是栈顶状态，把A和goto[S', A]分别压入符号栈和状态栈的栈顶;
		输出产生式A -> β;
	} else if (action[S, a] = accept) return;
	else error();
} while (1);
```
