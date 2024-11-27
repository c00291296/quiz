//Author: Ihor Antonov
//Student Number: C00291296
#include<iostream>
#include<fstream>
#include<cstdlib>
#include<vector>
#include<ctime>
#include<string>
#include<algorithm>
#include<random>
#include<map>
//graphics. You can build and install raylib system-wide with "cmake . && make && sudo make install"
//and everything just werks and you can use <raylib.h> instead of "raylib.h", just dont
//forget to install the dependencies ;^)
#include<raylib.h>
#define RAYGUI_IMPLEMENTATION
#include"raygui.h"
//json includes
#include"rapidjson/document.h"
#include"rapidjson/writer.h"
#include"rapidjson/stringbuffer.h"


#define WWIDTH 800
#define WHEIGHT 600

using namespace std;
using namespace rapidjson;

class Question {
	private:
		string text;
		int correct_answer;
		vector<string> answers;
	public:
		Question(string t, int ca, vector<string> ans);
		int print();
		bool correct(int answer);
		int test_user();
};

bool Question::correct(int answer) {
	return answer == correct_answer;
}

Question::Question(string t, int ca, vector<string> ans)
{
	text = t;
	correct_answer = ca;
	answers = ans;
}

int Question::print()
{
	GuiTextBox((Rectangle){(WWIDTH-300)/2, 50, 300, 50,}, (char*)text.c_str(), 12, false);
	int i = 1;
	for(string ans : answers) {
		if(GuiButton(Rectangle{(WWIDTH-200)/2, 105+35*i, 200, 30}, ans.c_str()))
			return i;
		i++;
	}
	return -1;
}

int Question::test_user()
{
	int ans = print();
	return ans;
}

class ScoreTable {
	private:
		map<string, int> scores;
	public:
		void addScore(string name, int score);
		void load();
		void save();
		void display();
};

void ScoreTable::addScore(string name, int score) 
{
	if(!name.length() == 0) {
		scores[name] = score;
		//cout<<"Added "<<name<<" to HS table;"<<endl;
	}
}

void ScoreTable::load()
{
	ifstream sf("scores.json");
	string filetext = "";
	Document d;
	while(!sf.eof()) {
		char c = sf.get();
		if (c== EOF)
			break;
		filetext += c;
	}
	sf.close();
	d.Parse(filetext.c_str());
	for(auto& p : d.GetObject()) {
		addScore(p.name.GetString(), p.value.GetInt());
	}
}

void ScoreTable::save()
{
	ofstream sf("scores.json");
	Document d;
	d.Parse("{}");
	for(auto pair : scores) {
		Value name(pair.first.c_str(), pair.first.size(), d.GetAllocator());
		d.AddMember(name, pair.second, d.GetAllocator());

	}
	StringBuffer b;
	Writer<StringBuffer> w(b);
	d.Accept(w);
	
	sf<<b.GetString()<<endl;
	sf.close();
}

void ScoreTable::display()
{
	GuiTextBox((Rectangle){(WWIDTH-300)/2, 50, 300, 50},"************ THE HIGH SCORE TABLE! ************",16,false);
	vector<pair<string, int>> sv;
	for(auto kv:scores) {
		sv.push_back(kv);
	}
	sort(sv.begin(), sv.end(), [](pair<string,int>& a, pair<string,int>& b){return a.second>b.second;});
	int i = 0;
	for(auto pair : sv) {
		GuiTextBox((Rectangle){10, 105+i*30, 100, 25}, (char*)pair.first.c_str(), 12, false);
		DrawRectangle(140, 105+i*30, WWIDTH-140-10, 25, RED);
		DrawRectangle(140, 105+i*30, pair.second*(WWIDTH-140-10)/500, 25, GREEN);
		i++;
	}
}

		
//GAME STATES
#define GS_QUESTION 0
#define GS_RESULT 1
#define GS_ENTERNAME 2
#define GS_SCOREBOARD 3

int main(int argc, char** argv)
{
	char name[16];
	vector<string> answers;
	vector<Question> questions;
	//open file
	ifstream qdata("questions.json");
	string filetext = "";
	Document qdoc;
	while(!qdata.eof()) {
		char c = qdata.get();
		if (c== EOF)
			break;
		filetext += c;
	}
	qdata.close();
	qdoc.Parse(filetext.c_str());
	cout<<filetext;
	for(auto &o : qdoc.GetArray()) {
		string question = o["question"].GetString();
		int answer = o["answer"].GetInt();
		vector<string> ans;
		for(auto &a : o["answers"].GetArray()) {
			ans.push_back(a.GetString());
		}
		questions.push_back(Question(question, answer, ans));
	}

	int score = 0;
	//randomize question order
	random_device rnd;
	shuffle(questions.begin(), questions.end(), rnd);
	// TIME FOR THE GRAPHICS
	InitWindow(WWIDTH, WHEIGHT, "QUIZ!!! VERY FUN! COMPLETE IT!");
	int question_number = 0;
	int game_state = GS_ENTERNAME;
	bool question_success = false;
	Question q = questions[question_number];
	int response;
	ScoreTable hs;
	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
/*
		for (Question q : questions) {
			if(q.test_user())
				score+=100;
		}
		cout<<"CONGRATULATIONS, YOU EARNED "<<score<<" POINTS!"<<endl;
		*/
		switch(game_state) {
			case GS_QUESTION:

				response = q.test_user();
				if(response==-1)
					break;
				question_success = q.correct(response);
				if(question_success)
					score+=100;
				game_state = GS_RESULT;
				question_number++;
				if(question_number >= questions.size())
					game_state = GS_SCOREBOARD;
				else
					q = questions[question_number];
				break;
			case GS_RESULT:
				if(question_success)
					GuiTextBox((Rectangle){(WWIDTH-300)/2, 50, 300, 50}, "Correct!", 12, false);
				else
					GuiTextBox((Rectangle){(WWIDTH-300)/2, 50, 300, 50}, "Wrong!", 12, false);

				if(GuiButton((Rectangle){(WWIDTH-200)/2, 105, 200, 50}, "Continue"))
					game_state = GS_QUESTION;
				break;
			case GS_SCOREBOARD:
				hs.load();
				hs.addScore(string(name), score);
				hs.display();
				hs.save();
				break;
			case GS_ENTERNAME:
					GuiTextBox((Rectangle){(WWIDTH-300)/2, 50, 300, 50}, "PLEASE, ENTER NAME:", 12, false);
					GuiTextBox((Rectangle){(WWIDTH-300)/2, 105, 300, 50}, name, 12, true);
					if(GuiButton((Rectangle){(WWIDTH-200)/2, 160, 200, 50}, "Continue"))
						game_state = GS_QUESTION;

					break;
			default:
				break;
		}
		EndDrawing();
	}
	CloseWindow();

	return 0;
}
