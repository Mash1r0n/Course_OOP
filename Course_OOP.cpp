#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <ctime>
#include <limits>
#ifdef max
#undef max
#endif

using namespace std;

#define TextChange 1 //Означає відображення текстового редактору без команд
#define SimpleTextChange 2 //Означає відображення текстового редактору з командами
#define InputText 3 //Означає відображення текстового редактору з командами вставки тексту
#define CutText 4 //Означає відображення текстового редактору з командами вирізки тексту
#define DeleteText 5 //Означає відображення текстового редактору з командами видалення тексту
#define MoveText 6 //Означає відображення текстового редактору з командами переміщення тексту
#define CopyText 7 //Означає відображення текстового редактору з командами копіювання тексту

#define AllSessions 8 //Означає відображення всіх сесій
#define CompareHistory 9 //Означає порівняння з історії перед виконанням
#define FilesMenu 10 //Означає порівняння з історії перед виконанням
#define StartMenu 11 //Означає порівняння з історії перед виконанням
#define CompareSessions 12 //Означає порівняння з історії перед виконанням

//#define NameSaveFile "SessionSave.txt"

#define SortByLowest 0 //Індекс для сортування за убуванням
#define SortByHighest 1 //Індекс для сортування за зростанням

class Session;

class Memento {
public:
    virtual ~Memento() {}
    virtual string GetSessionName() const = 0;
    virtual string GetSessionTextLine() const = 0;
    virtual string GetSessionTauntEvent() const = 0;
};

class Snapshot : public Memento {
private:
    string SessionName;
    string SessionTextLine;
    string SessionTauntEvent;


public:
    Snapshot(string, string, string);

    

    string GetSessionName() const override {
        return SessionName;
    }

    string GetSessionTextLine() const override {
        return SessionTextLine;
    }

    string GetSessionTauntEvent() const override {
        return SessionTauntEvent;
    }
};

Snapshot::Snapshot(string Name, string TextLine, string TauntEvent) : SessionName(Name), SessionTextLine(TextLine), SessionTauntEvent(TauntEvent) {
}

class Session {
private:
    string SessionName;
    string TextLine;
    string LastTauntEvent;
public:
    string GetName() {
        return SessionName;
    }

    Session(string, string, string);

    

    string GetTextLine() {
        return TextLine;
    }

    Memento* Save() {
        return new Snapshot(SessionName, TextLine, LastTauntEvent);
    }

    void Restore(Memento*);

    void SetTauntEvent(string);

    void InputToText(int, string);

    

    void DeleteFromText(int, int);

    

    void SaveFile(string, bool);

    

};

void Session::SetTauntEvent(string NameOfTaunt) {
    LastTauntEvent = NameOfTaunt;
}

Session::Session(string SessionName, string TextLine = "", string TauntEvent = "") : SessionName(SessionName), TextLine(TextLine), LastTauntEvent(TauntEvent) {};

void Session::SaveFile(string NameSaveFile, bool Mode) {
    fstream SaveStream(NameSaveFile, Mode ? ios::out : ios::app);
    SaveStream << SessionName << "+" << TextLine;
    SaveStream.close();
}

void Session::DeleteFromText(int StartPos, int EndPos) {
    TextLine.erase(StartPos, EndPos);
}

void Session::InputToText(int Position, string Text) {
    TextLine.insert(Position, Text);
}

void Session::Restore(Memento* Backup) {
    SessionName = Backup->GetSessionName();
    TextLine = Backup->GetSessionTextLine();
    LastTauntEvent = Backup->GetSessionTauntEvent();
}

class Caretaker {
private:
    vector<Memento*> mementos_;

    Session* originator_;

public:
    Caretaker(Session*);
    Caretaker(Caretaker&);

    ~Caretaker() {
        for (auto memento : mementos_) delete memento;
    }

    string GetUndoTextLine(int);

    

    void Backup() {
        this->mementos_.push_back(this->originator_->Save());
    }

    void Undo() {
        if (!this->mementos_.size()) {
            return;
        }
        Memento* memento = this->mementos_.back();
        this->mementos_.pop_back();

        try {
            this->originator_->Restore(memento);
        }
        catch (...) {
            this->Undo();
        }
    }

    void MementosBackupFromFile(Snapshot*);

    vector<Memento*> GetActiveSessionMemento() {
        return mementos_;
    }

    void SaveHistory(string);

};

void Caretaker::MementosBackupFromFile(Snapshot* memento) {
    mementos_.push_back(memento);
}

void Caretaker::SaveHistory(string NameSaveFile) {
    fstream SaveStream(NameSaveFile, ios::app);

    if (mementos_.empty()) {
        SaveStream << endl;
        SaveStream.close();

        return;
    }

    for (Memento* TempMem : mementos_) {
        SaveStream << "~" << TempMem->GetSessionName() << "+" << TempMem->GetSessionTextLine() << "+" << TempMem->GetSessionTauntEvent();
    }

    SaveStream << "~" << endl; //Разобраться с тем, что сделать, если вдруг у меня нечего сохранять, тогда endl не сработает и всё слепится в 1 строку

    SaveStream.close();
}

string Caretaker::GetUndoTextLine(int UndoNum) {
    return mementos_[UndoNum > mementos_.size() - 1 ? mementos_.size() - 1 : mementos_.size() - UndoNum]->GetSessionTextLine();
}

Caretaker::Caretaker(Session* originator) : originator_(originator) {
}

Caretaker::Caretaker(Caretaker& copy) {
    mementos_ = copy.mementos_;
    originator_ = copy.originator_;
}

struct SavePattern {
    string SessionName;
    string TextLine;
};

struct SavePatternForMemento {
    string SessionName;
    string TextLine;
    string TauntEvent;
};

union RewiewAndCompare
{
    int UndoRewiewNumber;
    int CompareSessionIndex;
};

class Editor {
private:
    vector<pair<Session*, Caretaker*>> AvailableSessions;
    Session* ActiveSession;
    Caretaker* History;
    RewiewAndCompare R_and_C;
    string NameSaveFile;

    bool ShowAvailableSessions() {
        if (AvailableSessions.empty()) {
            return false;
        }
        Render(AllSessions);
        return true;
    }

    bool SetActiveSession(int SessionIndex) {
        if (SessionIndex <= AvailableSessions.size() - 1 && !(SessionIndex < 0)) {
            ActiveSession = AvailableSessions[SessionIndex].first;

            History = AvailableSessions[SessionIndex].second;

            return true;
        }
        else {
            return false;
        }
    }

    void SaveAllSessions() {
        if (!AvailableSessions.empty()) {
            AvailableSessions[0].first->SaveFile(NameSaveFile, true);
            AvailableSessions[0].second->SaveHistory(NameSaveFile);

            if (AvailableSessions.size() - 1 > 0) {
                for (int i = 1; i < AvailableSessions.size(); i++) {
                    AvailableSessions[i].first->SaveFile(NameSaveFile, false);
                    AvailableSessions[i].second->SaveHistory(NameSaveFile);
                }
            }

        }
    }
public:
    

    Editor() {
        Render(StartMenu);
        ActiveSession = NULL;
        R_and_C.UndoRewiewNumber = -1;
    }

    void ResetProgram() {
        ActiveSession = NULL;
        R_and_C.UndoRewiewNumber = -1;
        AvailableSessions.clear();
    }

    void SetNameSaveFile(string nameSaveFile) {
        NameSaveFile = nameSaveFile;
    }

    string GetNameSaveFile() { return NameSaveFile; }

    void SessionBackupFromSave() {
        fstream ReadStream(NameSaveFile, ios::in);

        string SaveLine;

        while (getline(ReadStream, SaveLine)) {
            SavePattern Example;
            istringstream AnotherStream(SaveLine);

            getline(AnotherStream, Example.SessionName, '+');
            getline(AnotherStream, Example.TextLine, '~');

            Session* TempSessionData = new Session(Example.SessionName, Example.TextLine);

            string MementoData;

            Caretaker* TempCaretaker = new Caretaker(TempSessionData);

            while (getline(AnotherStream, MementoData, '~')) {
                istringstream ParseMementoData(MementoData);

                SavePatternForMemento TempConcreteMemento;

                getline(ParseMementoData, TempConcreteMemento.SessionName, '+');
                getline(ParseMementoData, TempConcreteMemento.TextLine, '+');
                getline(ParseMementoData, TempConcreteMemento.TauntEvent, '~');

                TempCaretaker->MementosBackupFromFile(new Snapshot(TempConcreteMemento.SessionName, TempConcreteMemento.TextLine, TempConcreteMemento.TauntEvent));
            }

            AvailableSessions.push_back(make_pair(TempSessionData, TempCaretaker));

        }

        system("cls");
        cout << " < Сеанс було завантажено >" << endl;

        ReadStream.close();
    }

    void SessionsDelete(int FromIndex) {
        for (int i = AvailableSessions.size() - 1; i > FromIndex; i--) {
            delete AvailableSessions[i].first;
            delete AvailableSessions[i].second;

            AvailableSessions.erase(AvailableSessions.begin() + i);
            system("cls");

        }
    }

    void SaveAllSessionsForm() {
        system("cls");
        SaveAllSessions();
        cout << " < Дані було збережено у файл " << NameSaveFile << "!>" << endl;
        ShowAvailableSessionsForm();
    }

    void SessionSelector() {
        int Choice;
        
        костиль1:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль1;
        }

        switch (Choice) {
        case 0: {
            system("cls");
            Render(TextChange);
        }break;

        case 1: {
            SetActiveSessionCompareForm();
            Render(CompareSessions);
        }break;

        default: {
            system("cls");
            cout << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(TextChange);
        }
        }
    }

    void ShowAvailableSessionsForm() {
        if (!ShowAvailableSessions()) {
            cout << "!!! Немає доступних сесій !!!" << endl;
            Render(TextChange);
            return;
        }
        else {
            cout << "1 - Відкатитись до минулого сеансу\n0 - Назад" << endl << endl;
            cout << "Оберіть команду: ";
            SessionSelector();

        }
    }

    void SetActiveSessionForm() {
        cout << "Введіть номер сеансу: ";
        int SessionNumber;

        костиль3:

        if (!(cin >> SessionNumber)) {
            cout << "Неправильно введено номер сеансу, введіть правильно: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль3;
        }

        if (SetActiveSession(SessionNumber)) {
            system("cls");
            Render(TextChange);
        }

        else {
            system("cls");
            cout << "!!! Сеанса під таким індексом не існує !!!" << endl;
            ShowAvailableSessionsForm();
            return;
        }
    }

    void SetActiveSessionCompareForm() {
        cout << "Введіть номер сеансу: ";
        int SessionNumber;

    костиль5:

        if (!(cin >> SessionNumber)) {
            cout << "Неправильно введено номер сеансу, введіть правильно: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль5;
        }

        if (SetActiveSession(SessionNumber)) {
            system("cls");
            R_and_C.CompareSessionIndex = SessionNumber;
            Render(CompareSessions);
        }

        else {
            system("cls");
            cout << "!!! Сеанса під таким індексом не існує !!!" << endl;
            ShowAvailableSessionsForm();
            return;
        }
    }

    void CreateNewSession() {

        string Name = GetCurrentDateTime();

        string Text = "";

        system("cls");

        Session* session = new Session(Name, Text);
        Caretaker* caretaker = new Caretaker(session);

        AvailableSessions.push_back(make_pair(session, caretaker));
        SetActiveSession(AvailableSessions.size() - 1);
    }
    void CreateNewSession(string WithText, string WithName) {

        system("cls");

        Session* session = new Session(WithName, WithText);
        Caretaker* caretaker = new Caretaker(session);

        AvailableSessions.push_back(make_pair(session, caretaker));
        SetActiveSession(AvailableSessions.size() - 1);
    }

    void InputToText() {
        int Choice;

    костиль6:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль6;
        }

        switch (Choice) {
        case 0: {
            system("cls");
            Render(TextChange);
        }break;

        case 1: {
            string clipboardText;
            if (OpenClipboard(NULL)) {
                HANDLE hData = GetClipboardData(CF_TEXT);
                char* pszData = static_cast<char*>(GlobalLock(hData));
                if (pszData != NULL) {
                    clipboardText = pszData;
                }
                GlobalUnlock(hData);
                CloseClipboard();
            }

            int Pos;
            cout << " Введіть номер символу перед яким треба поставити текст: ";

        костиль7:

            if (!(cin >> Pos)) {
                cout << "Неправильно введена позиція, введіть ще раз: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                goto костиль7;
            }

            ActiveSession->SetTauntEvent("Введення");
            History->Backup();

            ActiveSession->InputToText(Pos, clipboardText);

            system("cls");
            Render(TextChange);

        }break;

        case 2: {
            string Text;
            cout << " Введіть текст, який треба вставити: ";
            cin >> Text;
            Text = Text.substr(0, 99);
            int Pos;
            cout << " Введіть номер символу перед яким треба поставити текст: ";

        костиль8:

            if (!(cin >> Pos)) {
                cout << "Неправильно введена позиція, введіть ще раз: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                goto костиль8;
            }

            ActiveSession->SetTauntEvent("Введення");
            History->Backup();

            ActiveSession->InputToText(Pos, Text);

            system("cls");
            Render(TextChange);

        }break;

        default: {
            system("cls");
            cout << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(TextChange);
        }

        }
    }

    void CutFromText() {
        int Choice;

    костиль9:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль9;
        }

        switch (Choice) {
        case 0: {
            system("cls");
            Render(TextChange);
        }break;

        case 1: {
            int Pos, Count;
            cout << "Введіть через пробіл номер символу з якого почати вирізання та кількість символів після нього: ";

        костиль10:

            if (!(cin >> Pos >> Count)) {
                cout << "Неправильно введена позиція або кількість символів після позиції, введіть ще раз: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                goto костиль10;
            }

            string newText = ActiveSession->GetTextLine().substr(Pos, Count);

            OpenClipboard(0);
            EmptyClipboard();
            HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, newText.size());
            if (!hg) {
                CloseClipboard();
                return;
            }
            memcpy(GlobalLock(hg), newText.c_str(), newText.size());
            GlobalUnlock(hg);
            SetClipboardData(CF_TEXT, hg);
            CloseClipboard();
            GlobalFree(hg);

            system("cls");

            //В будущем поправить!!! Потому что оно не отображается в буфере

            ActiveSession->SetTauntEvent("Вирізання");
            History->Backup();

            ActiveSession->DeleteFromText(Pos, Pos + Count + 1);

            Render(TextChange);

        }break;

        default: {
            system("cls");
            cout << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(TextChange);
        }
        }
    }

    void DeleteFromText() {
        int Choice;

    костиль11:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль11;
        }

        switch (Choice) {
        case 0: {
            system("cls");
            Render(TextChange);
        }break;

        case 1: {
            int StartPos, Count;
            cout << "Введіть через пробіл номер символу з якого почати видалення та кількість символів після нього: ";

        костиль12:

            if (!(cin >> StartPos >> Count)) {
                cout << "Неправильно введена позиція або кількість символів після позиції, введіть ще раз: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                goto костиль12;
            }

            ActiveSession->SetTauntEvent("Видалення");
            History->Backup();

            ActiveSession->DeleteFromText(StartPos, StartPos + Count + 1);

            system("cls");
            Render(TextChange);
        }break;

        default: {
            system("cls");
            cout << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(TextChange);
        }
        }
    }

    void MoveInText() {
        int Choice;

    костиль14:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль14;
        }

        switch (Choice) {
        case 0: {
            system("cls");
            Render(TextChange);
        }break;

        case 1: {
            int StartPos, Count, Pos;
            cout << "Введіть через пробіл номер символу з якого почати виділення тексту та кількість символів після нього: ";

        костиль15:

            if (!(cin >> StartPos >> Count)) {
                cout << "Неправильно введена позиція або кількість символів після позиції, введіть ще раз: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                goto костиль15;
            }

            string SubText = ActiveSession->GetTextLine().substr(StartPos, Count);

            ActiveSession->SetTauntEvent("Переміщення");
            History->Backup();

            ActiveSession->DeleteFromText(StartPos, StartPos + Count + 1);

            system("cls");
            Render(SimpleTextChange);

            cout << " Введіть номер символу перед яким треба поставити текст: ";

        костиль16:

            if (!(cin >> Pos)) {
                cout << "Неправильно введено номер символу, введіть ще раз: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                goto костиль16;
            }

            ActiveSession->InputToText(Pos, SubText);

            system("cls");
            Render(TextChange);

        }break;

        default: {
            system("cls");
            cout << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(TextChange);
        }
        }
    }

    void CopyInText() {
        int Choice;

    костиль17:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль17;
        }

        switch (Choice) {
        case 0: {
            system("cls");
            Render(TextChange);
        }break;

        case 1: {
            int Pos, Count;
            cout << "Введіть через пробіл номер символу з якого почати копіювання та кількість символів після нього: ";

        костиль18:

            if (!(cin >> Pos >> Count)) {
                cout << "Неправильно введена позиція або кількість символів після позиції, введіть ще раз: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                goto костиль18;
            }

            string newText = ActiveSession->GetTextLine().substr(Pos, Count);

            OpenClipboard(0);
            EmptyClipboard();
            HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, newText.size());
            if (!hg) {
                CloseClipboard();
                return;
            }
            memcpy(GlobalLock(hg), newText.c_str(), newText.size());
            GlobalUnlock(hg);
            SetClipboardData(CF_TEXT, hg);
            CloseClipboard();
            GlobalFree(hg);

            system("cls");

            //В будущем поправить!!! Потому что оно не отображается в буфере

            Render(TextChange);

        }break;

        default: {
            system("cls");
            cout << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(TextChange);
        }
        }
    }

    void TextChangeSelector() {
        int Choice;

    костиль19:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль19;
        }

        switch (Choice) {
        case 0: {
            CreateNewSession(ActiveSession->GetTextLine(), GetCurrentDateTime());
            SaveAllSessions();
            system("cls");
            ActiveSession = NULL;
            Render(FilesMenu);
        }break;

        case -1: {
            system("cls");
            ShowAvailableSessionsForm();
        }break;

        case 1: {
            system("cls");
            Render(InputText);
        }break;

        case 2: {
            system("cls");
            Render(CutText);
        }break;

        case 3: {
            system("cls");
            Render(DeleteText);
        }break;

        case 4: {
            system("cls");
            Render(MoveText);
        }break;

        case 5: {
            system("cls");
            Render(CopyText);
        }break;
        case 6: {
            system("cls");
            UndoSelector();
            Render(TextChange);
        }break;
        default: {
            system("cls");
            cout << right << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(TextChange);
        }
        }
    }

    void UndoSelector() {
        cout << "1 - Видалити останню подію      2 - Видалити останні декілька подій";
        cout << endl << "Оберіть команду: ";
        int Choice;

    костиль20:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль20;
        }

        switch (Choice) {
        case 1: {
            History->Undo();
            Render(TextChange);
        }break;

        case 2: {
            int Number;
            cout << "Скільки подій ви бажаєте відмінити: ";

        костиль21:

            if (!(cin >> Number)) {
                cout << "Неправильно введено кількість подій, введіть ще раз: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                goto костиль21;
            }

            system("cls");
            R_and_C.UndoRewiewNumber = Number;
            Render(CompareHistory);
            cout << endl << endl << "Ви підтверджуєте відміну (Так): ";
            string Answer;
            cin >> Answer;
            if (Answer == "Так") {
                system("cls");

                for (int i = 0; i < Number; i++) {
                    History->Undo();
                }

                Render(TextChange);
            }
            else {
                system("cls");
                Render(TextChange);
            }

        }break;

        default: {
            system("cls");
            cout << right << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(TextChange);
        }
        }
    }

    vector<pair<string, string>> getCOIFiles() {
        vector<pair<string, string>> files;
        auto path = filesystem::current_path();

        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.path().extension() == ".coi") {
                files.push_back(make_pair(entry.path().filename().string(), entry.path().string()));
            }
        }

        return files;
    }

    void NewFileSelector() {
        ResetProgram();
        int Choice;

    костиль23:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль23;
        }

        switch (Choice) {
        case 0: {
            system("cls");
            Render(StartMenu);
        }break;

        case 1: {
            system("cls");
            CreateNewFile();
            CreateNewSession();
            system("cls");
            Render(TextChange);
        }break;

        default: {
            system("cls");
            cout << right << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(FilesMenu);
        }
        }
    }

    void CreateNewFile() {
        cout << "Введіть назву файла: ";
        string Name;
        cin >> Name;
        fstream crt(Name += ".coi", ios::out);
        SetNameSaveFile(Name);
        crt.close();
    }

    void ChooseTheFile(int Index) {
        vector<pair<string, string>> Paths = getCOIFiles();
        SetNameSaveFile(Paths[Index].first);
    }

    

    bool DeleteIndexFile(string filepath) {
        if (remove(filepath.c_str()) != 0) {
            return false;
        }
        else {
            return true;
        }
    }

    bool DeleteTheFile(int Index) {
        vector<pair<string, string>> Paths = getCOIFiles();
        return DeleteIndexFile(Paths[Index].second);
    }

    void AlreadyHaveFileSelector() {
        ResetProgram();
        int Choice;

    костиль25:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль25;
        }

        switch (Choice) {
        case 0: {
            system("cls");
            Render(StartMenu);
        }break;

        case 1: {
            system("cls");
            CreateNewFile();
            CreateNewSession();
            system("cls");
            Render(TextChange);
        }break;

        case 2: {
            cout << " Оберіть індекс файлу: ";
            int Index;

        костиль26:

            if (!(cin >> Index)) {
                cout << "Неправильно введено індекс, введіть ще раз: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                goto костиль26;
            }

            ChooseTheFile(Index);
            system("cls");
            SessionBackupFromSave();
            SetActiveSession(AvailableSessions.size() - 1);
            Render(TextChange);
        }break;

        case 3: {
            cout << " Оберіть індекс файлу: ";
            int Index;

        костиль27:

            if (!(cin >> Index)) {
                cout << "Неправильно введено індекс, введіть ще раз: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                goto костиль27;
            }

            system("cls");

            if (DeleteTheFile(Index)) {
                cout << "< Файл було успішно видалено >" << endl;
            }
            else {
                cout << "!!! Помилка видалення файлу !!!" << endl;
            }

            Render(FilesMenu);
        }break;

        default: {
            system("cls");
            cout << right << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(FilesMenu);
        }

        }
    }

    void StartMenuSelector(){
        int Choice;

    костиль28:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль28;
        }

        switch (Choice) {
        case 0: {
            exit(1);
        }break;

        case 1: {
            system("cls");
            Render(FilesMenu);
        }break;


        default: {
            system("cls");
            cout << right << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(StartMenu);
        }
        }
    }

    string GetCurrentDateTime() {
        auto t = time(nullptr);
        tm tm;
        localtime_s(&tm, &t);

        ostringstream oss;
        oss << put_time(&tm, "%Y-%m-%d %H:%M");
        auto str = oss.str();

        return str;
    }

    void CompareSessionSelector() {
        int Choice;

    костиль29:

        if (!(cin >> Choice)) {
            cout << "Неправильно введена команда, введіть ще раз: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            goto костиль29;
        }

        switch (Choice) {
            case 0: {
                system("cls");
                SetActiveSession(AvailableSessions.size() - 1);
                Render(TextChange);
            }break;

            case 1: {
                system("cls");
                SessionsDelete(R_and_C.CompareSessionIndex);
                SaveAllSessions();
                SetActiveSession(AvailableSessions.size() - 1);
                Render(TextChange);
            }break;


            default: {
                system("cls");
                cout << right << " !!! Ви обрали неіснуючу дію !!!" << endl;
                Render(StartMenu);
            }
        }
    }

    void Render(int Variative) {
        if (Variative == StartMenu) {
            cout << "1 - Переглянути файли      0 - Вихід з програми" << endl;
            cout << "оберіть команду: ";
            StartMenuSelector();
        }
        else if (Variative == FilesMenu) {
            vector<pair<string, string>> Paths = getCOIFiles();
            if (!Paths.empty()) {
                cout << "Список доступних файлів:" << endl;
                cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;

                for (int i = 0; i < Paths.size(); i++) {
                    cout << "| " << setw(4) << setprecision(4) << setfill(' ') << left << i << setw(112) << setprecision(112) << setfill(' ') << Paths[i].first.substr(0, 111) << "|" << endl;
                }

                cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;
                cout << "1 - Створити файл      2 - Обрати файл    3 - Видалити файл     0 - Перейти до початку" << endl;
                cout << endl << " Оберіть команду: ";
                AlreadyHaveFileSelector();
            }
            else {
                cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;
                cout << "|                                                                                                                     |" << endl;
                cout << "|                                                                                                                     |" << endl;
                cout << "|                                               Створіть файл з даними!                                               |" << endl;
                cout << "|                                                                                                                     |" << endl;
                cout << "|                                                                                                                     |" << endl;
                cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;
                cout << "1 - Створити файл      0 - Перейти до початку" << endl;
                cout << endl << " Оберіть команду: ";
                NewFileSelector();
            }
        }

        else if (Variative >= TextChange && Variative <= CopyText) {

            vector<Memento*> TempMemento = History->GetActiveSessionMemento();

            int CountCoef = TempMemento.size() > 5 ? TempMemento.size() - 5 : 0;


            /*cout << "[Ім'я сесії: " << ActiveSession->GetName() << "]" << endl;*/
            cout << "Ім'я файлу: " << GetNameSaveFile() << endl;
            cout << " ----------------------------------------------------------------------------------------------------" << "------------------" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 5 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 5 ? (*(TempMemento.begin() + 4 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 4 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 4 ? (*(TempMemento.begin() + 3 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|" << setw(100) << setprecision(100) << setfill(' ') << left << ActiveSession->GetTextLine().substr(0, 99) << "|" << setw(3) << setprecision(3) << setfill(' ') << left << 3 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 3 ? (*(TempMemento.begin() + 2 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 2 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 2 ? (*(TempMemento.begin() + 1 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 1 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 1 ? (*(TempMemento.begin() + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << " ----------------------------------------------------------------------------------------------------" << "------------------" << endl;
            switch (Variative) {
            case TextChange: {
                cout << "1 - Вставити      2 - Вирізати      3 - Видалити     4 - Перемістити     5 - Скопіювати     6 - Відмінити     \n-1 - Переглянути історію сеансів     0 - Вихід" << endl;
                cout << endl << " Оберіть команду: ";
                TextChangeSelector();
            }break;

            case InputText: {
                cout << "1 - Вставити останні символи з буферу      2 - Ввести символи для вставки      0 - Вихід" << endl;
                cout << endl << " Оберіть команду: ";
                InputToText();
            };
            case CutText: {
                cout << "1 - Ввести номер початкового символу та кількість символів після початку для вирізки      0 - Вихід" << endl;
                cout << endl << " Оберіть команду: ";
                CutFromText();
            }break;

            case DeleteText: {
                cout << "1 - Ввести номер початкового символу та кількість символів після початку для видалення      0 - Вихід" << endl;
                cout << endl << " Оберіть команду: ";
                DeleteFromText();
            }break;

            case MoveText: {
                cout << "1 - Перемістити текст за номером початку, кількістю символів та індексу місця вставки      0 - Вихід" << endl;
                cout << endl << " Оберіть команду: ";
                MoveInText();
            }break;

            case CopyText: {
                cout << "1 - Скопіювати текст за номером початку і кількістю символів після нього      0 - Вихід" << endl;
                cout << endl << " Оберіть команду: ";
                CopyInText();
            }break;
            }

        }
        else if (Variative == AllSessions) {
            cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;

            for (int i = 0; i < AvailableSessions.size(); i++) {
                cout << "| " << setw(4) << setprecision(4) << setfill(' ') << left << i << setw(112) << setprecision(112) << setfill(' ') << AvailableSessions[i].first->GetName().substr(0, 111) << "|" << endl;
            }

            cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;
        }

        else if (Variative == CompareHistory) {
            
            cout << "Поточний стан:" << endl;
            cout << " ----------------------------------------------------------------------------------------------------------------------" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|" << setw(100) << setprecision(100) << setfill(' ') << left << ActiveSession->GetTextLine().substr(0, 99) << "                 |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << " ----------------------------------------------------------------------------------------------------------------------" << endl;
        
            

            cout << endl << endl << endl<< "Після відміни дій:" << endl;
            cout << " ----------------------------------------------------------------------------------------------------------------------" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|" << setw(100) << setprecision(100) << setfill(' ') << left << History->GetUndoTextLine(R_and_C.UndoRewiewNumber).substr(0, 99) << "                 |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << " ----------------------------------------------------------------------------------------------------------------------" << endl;
        }
        else if (Variative == CompareSessions) {
            vector<Memento*> TempMemento = History->GetActiveSessionMemento();

            int CountCoef = TempMemento.size() > 5 ? TempMemento.size() - 5 : 0;

            cout << "[Перегляд сесії: " << ActiveSession->GetName() << "]" << endl;
            cout << "Ім'я файлу: " << GetNameSaveFile() << endl;
            cout << " ----------------------------------------------------------------------------------------------------" << "------------------" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 5 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 5 ? (*(TempMemento.begin() + 4 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 4 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 4 ? (*(TempMemento.begin() + 3 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|" << setw(100) << setprecision(100) << setfill(' ') << left << ActiveSession->GetTextLine().substr(0, 99) << "|" << setw(3) << setprecision(3) << setfill(' ') << left << 3 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 3 ? (*(TempMemento.begin() + 2 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 2 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 2 ? (*(TempMemento.begin() + 1 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 1 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 1 ? (*(TempMemento.begin() + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << " ----------------------------------------------------------------------------------------------------" << "------------------" << endl;
            
            cout << "1 - Виконати відкатування      0 - Вихід" << endl;
            cout << endl << " Оберіть команду: ";
            CompareSessionSelector();
        }
    }
};

void main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    Editor MainEditor = Editor();

    return;
}