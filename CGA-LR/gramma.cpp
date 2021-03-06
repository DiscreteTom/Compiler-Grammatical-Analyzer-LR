#include "gramma.h"
#include <QStack>
#include "myutility.h"
#include <iostream>

using namespace std;

void GrammaTable::killBlank(QString &str) const
{
	QString result;
	for (auto c : str)
	{
		if (c != ' ' && c != '\t' && c != '\n')
			result += c;
	}
	str = result;
}

bool GrammaTable::format(QString &str) const
{
	killBlank(str);
	//check format
	int i = str.indexOf('-');
	if (str[i + 1] != '>')
		return false;
	return true;
}

void GrammaTable::killDuplicated(int index)
{
	if (index == -1)
	{
		// eliminate same candidate for all gramma
		for (int i = 0; i < grammas.size(); ++i)
		{
			killDuplicated(i);
		}
	}
	else
	{
		// eliminate same candidate for grammas[index]
		for (int i = 0; i < grammas[index].size(); ++i)
		{
			for (int j = i + 1; j < grammas[index].size(); ++j)
			{
				if (grammas[index][i] == grammas[index][j])
				{
					// eliminate grammas[index][j]
					grammas[index].erase(grammas[index].begin() + j);
					--j;
				}
			}
		}
	}
}

void GrammaTable::killEpsilon()
{
	for (auto &gramma : grammas)
	{
		for (auto &candidate : gramma)
		{
			for (int i = 0; i < candidate.size(); ++i)
			{
				if (candidate[i] == EPSILON && candidate.size() > 1)
				{
					candidate.erase(candidate.begin() + i);
					--i;
				}
			}
		}
	}
}

void GrammaTable::getFirsts()
{
	firsts.clear();
	// add empty set
	for (int i = 0; i < grammas.size(); ++i)
	{
		firsts.push_back(First());
	}

	bool changed = true;
	while (changed)
	{
		changed = false;
		for (int i = 0; i < grammas.size(); ++i) // for each gramma
		{

			for (int j = 0; j < grammas[i].size(); ++j) // for each candidate
			{
				for (auto symbol : grammas[i][j])
				{
					if (symbol.type == Symbol::SymbolType::T)
					{
						// add this terminator, even EPSILON
						if (firsts[i].find(symbol) == firsts[i].end())
						{
							changed = true;
							firsts[i].insert(symbol);
						}
						break; // to next candidate
					}
					else // symbol.type == NT
					{
						for (auto first : firsts[symbol.index])
						{
							if (firsts[i].find(first) == firsts[i].end())
							{
								changed = true;
								firsts[i].insert(first);
							}
						}
						if (firsts[symbol.index].find(EPSILON) == firsts[symbol.index].end())
						{
							break; // to next candidate
						}
					}
				}
			}
		}
	}
}

void GrammaTable::getFollows()
{
	follows.clear();
	// add empty set
	for (int i = 0; i < grammas.size(); ++i)
	{
		follows.push_back(Follow());
	}

	// add END
	follows[0].insert(END);

	bool changed = true;
	while (changed)
	{
		changed = false;
		for (int i = 0; i < grammas.size(); ++i) // for each gramma
		{
			for (int j = 0; j < grammas[i].size(); ++j) // for each candidate
			{
				for (int k = 0; k < grammas[i][j].size(); ++k) // for each symbol
				{
					if (grammas[i][j][k].type == Symbol::SymbolType::NT) // this is an NT
					{
						if (k + 1 < grammas[i][j].size()) // next symbol exist
						{
							if (grammas[i][j][k + 1].type == Symbol::SymbolType::T) // next symbol is a terminator(it will not be END or EPSILON)
							{
								if (follows[grammas[i][j][k].index].find(grammas[i][j][k + 1]) == follows[grammas[i][j][k].index].end())
								{
									changed = true;
									follows[grammas[i][j][k].index].insert(grammas[i][j][k + 1]);
								}
								break; // to next candidate
							}
							else // next symbol is a NT
							{
								bool hasEPSILON = false;
								for (int x = k + 1; x < grammas[i][j].size(); ++x)
								{
									for (auto first : firsts[grammas[i][j][k + 1].index])
									{
										if (first != EPSILON && follows[grammas[i][j][k].index].find(first) == follows[grammas[i][j][k].index].end())
										{
											changed = true;
											follows[grammas[i][j][k].index].insert(first);
										}
										else if (first == EPSILON)
										{
											hasEPSILON = true;
										}
									}
									if (!hasEPSILON)
										break;
								}
								if (hasEPSILON) // the last NT in this candidate has EPSILON in its FIRST
								{
									for (auto follow : follows[i])
									{
										if (follows[grammas[i][j][k].index].find(follow) == follows[grammas[i][j][k].index].end())
										{
											changed = true;
											follows[grammas[i][j][k].index].insert(follow);
										}
									}
								}
							}
						}
						else //next symbol not exist
						{
							for (auto follow : follows[i])
							{
								if (follows[grammas[i][j][k].index].find(follow) == follows[grammas[i][j][k].index].end())
								{
									changed = true;
									follows[grammas[i][j][k].index].insert(follow);
								}
							}
						}
					}
				}
			}
		}
	}
}

void GrammaTable::getDFA()
{
	states.clear();
	dfa.clear();

	// get the first state
	State firstState;
	firstState.push_back({0, 0, 0});
	getState(firstState); // construct the whole state
	states.push_back(firstState);

	// now try to expand dfa
	for (int i = 0; i < states.size(); ++i)
	{
		State state = states[i];
		QVector<State> tStates;
		DFA tDfa;
		// expand
		for (int j = 0; j < state.size(); ++j)
		{
			// for each project
			auto project = state[j];
			if (project.index < grammas[project.ntIndex][project.candidateIndex].size())
			{
				// pointer can move right
				auto sym = grammas[project.ntIndex][project.candidateIndex][project.index]; // store symbol
				++project.index;
				if (!tDfa.contains({state, sym}))
				{
					// construct a new state
					State t;
					t.push_back(project);
					tStates.push_back(t);
					tDfa.insert({state, sym}, t);
				}
				else
				{
					// add this project to that state
					DFA_Key key = {state, sym};
					int index = tStates.indexOf(tDfa[key]);
					tStates[index].push_back(project);
					tDfa[key] = tStates[index];
				}
			}
		}
		// every project in state has been added to tStates
		// construct tStates and tDfa
		for (auto &s : tStates)
		{
			auto key = tDfa.key(s);
			getState(s);
			tDfa[key] = s;
		}

		// check duplicated state from tDfa and dfa, merge dfa and tDfa
		auto keys = tDfa.keys();
		for (auto key : keys)
		{
			int index = states.indexOf(tDfa[key]);
			if (index == -1)
			{
				states.push_back(tDfa[key]);
				dfa.insert(key, tDfa[key]);
			}
			else
			{
				dfa.insert(key, states[index]);
			}
		}
	}
}

void GrammaTable::getState(State &state)
{
	// make sure that state has at least one project
	bool flag = true; // flag of loop, should be false if there is no change after a loop
	while (flag)
	{
		flag = false;
		for (int i = 0; i < state.size(); ++i)
		{
			auto p = state.begin() + i;
			// for each project
			if (p->index >= grammas[p->ntIndex][p->candidateIndex].size())
				continue;
			if (grammas[p->ntIndex][p->candidateIndex][p->index].type == Symbol::SymbolType::NT)
			{
				// this is an NT, should add all its candidates
				int ntIndex = grammas[p->ntIndex][p->candidateIndex][p->index].index;
				for (int j = 0; j < grammas[ntIndex].size(); ++j)
				{
					Project t = {ntIndex, j, 0};
					if (!state.contains(t))
					{
						flag = true;
						i = -1; // restart loop
						state.push_back(t);
					}
				}
			}
		}
	}
}

int GrammaTable::getIndex(int ntIndex, int candidateIndex) const
{
	int result = 0;
	for (int i = 0; i < ntIndex; ++i)
	{
		result += grammas[i].size();
	}
	return result + candidateIndex;
}

void GrammaTable::getSLR_Table()
{
	// get reduce and accept
	for (auto state : states)
	{
		int ntIndex;
		int candidateIndex;
		int reduceIndex = getReduceIndex(state, ntIndex, candidateIndex);
		if (ntIndex != -1 && candidateIndex != -1)
		{
			//reduce
			for (auto s : follows[ntIndex])
			{
				Action a;
				a.index = reduceIndex;
				if (reduceIndex == 0)
				{
					a.type = Action::ActionType::Accept;
				}
				else
				{
					a.type = Action::ActionType::Reduce;
				}
				slrTable.insert({state, s}, a);
			}
		}
	}
	auto keys = dfa.keys();
	for (auto key : keys)
	{
		if (key.s.type == Symbol::SymbolType::T)
		{
			// shift
			slrTable.insert(key, {Action::ActionType::Shift, states.indexOf(dfa[key])});
		}
		else
		{
			// this is a non-terminator, action.type = goto
			slrTable.insert(key, {Action::ActionType::Goto, states.indexOf(dfa[key])});
		}
	}
}

int GrammaTable::getReduceIndex(const State &s, int &ntIndex, int &candidateIndex) const
{
	int result = 0;
	ntIndex = candidateIndex = -1;
	for (auto p : s)
	{
		if (p.index == grammas[p.ntIndex][p.candidateIndex].size())
		{
			ntIndex = p.ntIndex;
			candidateIndex = p.candidateIndex;
			break;
		}
	}
	if (ntIndex != -1 && candidateIndex != -1)
	{
		for (int i = 0; i < ntIndex; ++i)
		{
			result += grammas[i].size();
		}
		result += candidateIndex;
	}
	return result;
}

void GrammaTable::getCandidateIndex(int index, int &ntIndex, int &candidateIndex) const
{
	ntIndex = candidateIndex = -1;
	for (int i = 0; i < grammas.size(); ++i)
	{
		if (index >= grammas[i].size())
		{
			index -= grammas[i].size();
			continue;
		}
		else
		{
			ntIndex = i;
			candidateIndex = index;
			return;
		}
	}
}

int GrammaTable::candidateCount() const
{
	int result = 0;
	for (const auto &gramma : grammas)
	{
		result += gramma.size();
	}
	return result;
}

First GrammaTable::getFirst(const Candidate &candidate) const
{
	First result;
	for (auto symbol : candidate)
	{
		if (symbol.type == Symbol::SymbolType::T)
		{
			// add this terminator, even EPSILON
			if (result.find(symbol) == result.end())
			{
				result.insert(symbol);
			}
			return result;
		}
		else // symbol.type == NT
		{
			for (auto first : firsts[symbol.index])
			{
				if (result.find(first) == result.end())
				{
					result.insert(first);
				}
			}
			if (firsts[symbol.index].find(EPSILON) == firsts[symbol.index].end())
			{
				return result;
			}
		}
	}
	return result;
}

Candidate GrammaTable::parseInputToCandidate(const QString &str, QVector<int> *values) const
{
	int i = 0;
	Candidate result;
	while (i < str.length())
	{
		QString sym;
		int value = 0;
		if (str[i] == '$')
		{
			while (i + 1 < str.length() && str[i + 1] != '$')
			{
				++i;
				if (values)
				{
					value *= 10;
					value += str[i].toLatin1() - '0';
				}
				else
					sym += str[i];
			}
			if (i == str.length() || (sym.length() < 2 && !values))
			{
				// no matched $
				result.clear();
				return result;
			}
			else // str[i + 1] == $
			{
				++i;
			}
			if (values)
				sym = "num";
			int index = tTable.getIndex(sym, false);
			if (index == -1)
			{
				result.clear();
				return result;
			}
			result.push_back(Symbol({Symbol::SymbolType::T, index}));
			if (values)
				values->push_back(value);
			sym = "";
			value = 0;
		}
		else
		{
			// see other chars as terminator
			sym += str[i];
			while (i + 1 < str.length() && str[i] == '\'')
			{
				++i;
				sym += str[i];
			}
			int index = tTable.getIndex(sym, false);
			if (index == -1)
			{
				result.clear();
				return result;
			}
			result.push_back(Symbol({Symbol::SymbolType::T, index}));
			if (values)
				values->push_back(0);
		}
		++i;
	}
	return result;
}

int GrammaTable::insert(const QString &grammaLine)
{
	if (error)
		return lineCount;
	++lineCount;
	QString str = grammaLine;
	if (!format(str))
	{
		error = true;
		return lineCount;
	}
	if (!str.length())
	{
		error = true;
		return lineCount; // ERROR
	}

	// get left Symbol QString
	QString left;
	left += str[0];
	int i = 1; // index of str
	while (i < str.length() && str[i] == '\'')
	{
		left += str[i];
		++i;
	}

	// check left Symbol
	int grammaIndex = ntTable.getIndex(left);
	if (grammaIndex == grammas.size()) //new symbol
	{
		grammas.push_back(Gramma());
	}

	// get right
	i += 2; // read "->"
	Candidate candidate;
	QString sym; // current symbol QString
	while (i < str.length())
	{
		if (str[i] >= 'A' && str[i] <= 'Z')
		{
			sym += str[i];
			while (i + 1 < str.length() && str[i + 1] == '\'')
			{
				++i;
				sym += str[i];
			}
			//find this NT
			int index = ntTable.getIndex(sym);
			if (index == grammas.size()) // new NT
			{
				grammas.push_back(Gramma());
			}
			candidate.push_back({Symbol::SymbolType::NT, index});
			sym = "";
		}
		else if (str[i] == '$')
		{
			++i;
			while (i < str.length() && str[i] != '$')
			{
				sym += str[i];
				++i;
			}
			if (str[i] != '$')
			{
				// no matched $
				error = true;
				return lineCount;
			}
			else
			{
				// $ matched
				if (sym.length() < 2)
				{
					error = true;
					return lineCount;
				}
				else
				{
					// got a terminator
					int index = tTable.getIndex(sym);
					candidate.push_back({Symbol::SymbolType::T, index});
					sym = "";
				}
			}
		}
		else if (str[i] == '|')
		{
			if (candidate.size())
			{
				grammas[grammaIndex].push_back(candidate);
				candidate.clear();
			}
		}
		else
		{
			//other characters, inlcude '~', see them as terminator
			sym = str[i];
			while (i + 1 < str.length() && str[i + 1] == '\'')
			{
				sym += str[i + 1];
				++i;
			}
			int index = tTable.getIndex(sym);
			candidate.push_back({Symbol::SymbolType::T, index});
			sym = "";
		}
		++i;
	}
	if (candidate.size())
	{
		grammas[grammaIndex].push_back(candidate);
	}
	killDuplicated(grammaIndex);
	return 0;
}

bool GrammaTable::generate()
{
	if (error)
		return false;
	getFirsts();
	getFollows();
	getDFA();
	getSLR_Table();
	return true;
}

void GrammaTable::outputSingleCandidate(int ntIndex, int candidateIndex) const
{
	cout << ntTable.getStr(ntIndex) << " -> ";
	for (auto symbol : grammas[ntIndex][candidateIndex])
	{
		if (symbol.type == Symbol::SymbolType::NT)
		{
			cout << ntTable.getStr(symbol.index);
		}
		else
		{
			cout << tTable.getStr(symbol.index);
		}
	}
}

void GrammaTable::outputProject(const Project &p) const
{
	cout << ntTable.getStr(p.ntIndex) << " -> ";
	for (int i = 0; i < grammas[p.ntIndex][p.candidateIndex].size(); ++i)
	{
		if (i == p.index)
			cout << ".";
		if (grammas[p.ntIndex][p.candidateIndex][i].type == Symbol::SymbolType::NT)
		{
			cout << ntTable.getStr(grammas[p.ntIndex][p.candidateIndex][i].index);
		}
		else
		{
			cout << tTable.getStr(grammas[p.ntIndex][p.candidateIndex][i].index);
		}
	}
	if (p.index == grammas[p.ntIndex][p.candidateIndex].size())
		cout << ".";
}

void GrammaTable::outputSymbol(const Symbol &s) const
{
	if (s.type == Symbol::SymbolType::T)
		cout << tTable.getStr(s.index);
	else
		cout << ntTable.getStr(s.index);
}

void GrammaTable::outputSLR_Key(const SLR_Key &key) const
{
	cout << states.indexOf(key.state) << " + '";
	outputSymbol(key.s);
	cout << "'";
}

void GrammaTable::outputAction(const Action &a) const
{
	switch (a.type)
	{
	case Action::ActionType::Accept:
		cout << "ACC";
		break;
	case Action::ActionType::Goto:
		cout << "GOTO " << a.index;
		break;
	case Action::ActionType::Reduce:
		cout << "R" << a.index;
		break;
	case Action::ActionType::Shift:
		cout << "S" << a.index;
		break;
	default:
		break;
	}
}

void GrammaTable::output() const
{
	if (error)
	{
		cout << "Can NOT parse gramma to LR gramma\n";
		return;
	}

	cout << "Format gramma:\n";
	for (int i = 0; i < grammas.size(); ++i)
	{
		if (grammas[i].size())
			cout << ntTable.getStr(i) << " -> ";
		for (int j = 0; j < grammas[i].size(); ++j)
		{
			// each candidate
			for (int k = 0; k < grammas[i][j].size(); ++k)
			{
				// each symbol
				if (grammas[i][j][k].type == Symbol::SymbolType::NT)
				{
					cout << ntTable.getStr(grammas[i][j][k].index);
				}
				else // type == T
				{
					cout << tTable.getStr(grammas[i][j][k].index);
				}
			}
			if (j != grammas[i].size() - 1)
				cout << " | ";
		}
		if (grammas[i].size())
			cout << endl;
	}
	cout << endl;

	cout << "Candidates with index:\n";
	int index = 0;
	for (int i = 0; i < grammas.size(); ++i)
	{
		for (int j = 0; j < grammas[i].size(); ++j)
		{
			cout << index << "\t";
			outputSingleCandidate(i, j);
			++index;
			cout << endl;
		}
	}
	cout << endl;

	cout << "First sets:\n";
	for (int i = 0; i < firsts.size(); ++i)
	{
		cout << "First(" << ntTable.getStr(i) << "): ";
		for (auto first : firsts[i])
		{
			cout << tTable.getStr(first.index) << " ";
		}
		cout << "\n";
	}
	cout << endl;

	cout << "Follow sets:\n";
	for (int i = 0; i < firsts.size(); ++i)
	{
		cout << "Follow(" << ntTable.getStr(i) << "): ";
		for (auto follow : follows[i])
		{
			cout << tTable.getStr(follow.index) << " ";
		}
		cout << "\n";
	}
	cout << endl;

	cout << "LR(0) DFA states:\n";
	for (int i = 0; i < states.size(); ++i)
	{
		cout << "State[" << i << "]:\n";
		for (auto project : states[i])
		{
			cout << '\t';
			outputProject(project);
			cout << endl;
		}
	}
	cout << endl;

	cout << "LR(0) DFA transition functions:\n";
	auto keys = dfa.keys();
	for (auto key : keys)
	{
		cout << states.indexOf(key.state) << " + '";
		outputSymbol(key.s);
		cout << "' -> " << states.indexOf(dfa[key]);
		cout << endl;
	}
	cout << endl;

	cout << "SLR_1 table:\n";
	auto slrKeys = slrTable.keys();
	for (auto key : slrKeys)
	{
		outputSLR_Key(key);
		cout << " -> ";
		outputAction(slrTable[key]);
		cout << endl;
	}
}

bool GrammaTable::parse(const QString &str, bool calculateResult) const
{
	QVector<int> values;
	Candidate candidate;
	if (calculateResult)
		candidate = parseInputToCandidate(str, &values);
	else
		candidate = parseInputToCandidate(str);
	if (candidate.size() == 0)
	{
		cout << "Error input.\n";
		return false;
	}
	candidate.push_back(END);

	QStack<int> stateStack;
	QStack<Symbol> symbolStack;
	QStack<double> valueStack;
	stateStack.push(0);
	symbolStack.push(END);

	int index = 0; // index of candidate
	while (index < candidate.size())
	{
		SLR_Key key = {states[stateStack.top()], candidate[index]};
		if (!slrTable.contains(key))
			break;
		auto action = slrTable[key];
		outputAction(action);
		cout << endl;
		switch (action.type)
		{
		case Action::ActionType::Accept:
			cout << "Accepted.\n";
			if (calculateResult)
				cout << "Result is " << valueStack.top() << endl;
			return true;
			break;
		case Action::ActionType::Reduce:
		{
			// calculate value
			if (calculateResult)
			{
				double t = 0;
				switch (action.index)
				{
				case 1: // E -> E1 + T { E.v = E1.v + T.v }
					t = valueStack.top();
					valueStack.pop();
					valueStack.pop();
					t += valueStack.top();
					valueStack.pop();
					valueStack.push(t);
					break;
				case 2: // E -> E1 - T { E.v = E1.v - T.v }
					t = valueStack.top();
					valueStack.pop();
					valueStack.pop();
					t = valueStack.top() - t;
					valueStack.pop();
					valueStack.push(t);
					break;
				case 3: // E -> T { E.v = T.v }
					break;
				case 4: // T -> T1 * F { T.v = T1.v * F.v}
					t = valueStack.top();
					valueStack.pop();
					valueStack.pop();
					t *= valueStack.top();
					valueStack.pop();
					valueStack.push(t);
					break;
				case 5: // T -> T1 / F { T.v = T1.v / F.v}
					t = valueStack.top();
					valueStack.pop();
					valueStack.pop();
					t = valueStack.top() / t;
					valueStack.pop();
					valueStack.push(t);
					break;
				case 6: // T -> F { T.v = F.v }
					break;
				case 7: // F -> (E) { F.v = E.v }
					valueStack.pop();
					t = valueStack.top();
					valueStack.pop();
					valueStack.top() = t;
					break;
				case 8: // F -> num { F.v = num.v }
					break;
				default:
					break;
				}
			}
			int ntIndex;
			int candidateIndex;
			getCandidateIndex(action.index, ntIndex, candidateIndex);
			for (int i = 0; i < grammas[ntIndex][candidateIndex].size(); ++i)
			{
				stateStack.pop();
				symbolStack.pop();
			}
			SLR_Key gotoKey = {states[stateStack.top()], {Symbol::SymbolType::NT, ntIndex}};
			auto gotoAction = slrTable[gotoKey];
			outputAction(gotoAction); // output
			cout << endl;
			stateStack.push(gotoAction.index);
			symbolStack.push(ntTable[ntIndex]);
			--index; // do not increase index
			break;
		}
		case Action::ActionType::Shift:
			stateStack.push(action.index);
			symbolStack.push(candidate[index]);
			if (calculateResult)
				valueStack.push(values[index]);
			break;
		default:
			break;
		}
		++index;
	}
	cout << "This line not belongs to this gramma.\n";
	return false;
}
