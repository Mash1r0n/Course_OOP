#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <filesystem>


using namespace std;

#define MainMenu 0 //Означає відображення головного меню
#define TextChange 1 //Означає відображення текстового редактору без команд
#define SimpleTextChange 2 //Означає відображення текстового редактору з командами
#define InputText 3 //Означає відображення текстового редактору з командами вставки тексту
#define CutText 4 //Означає відображення текстового редактору з командами вирізки тексту
#define DeleteText 5 //Означає відображення текстового редактору з командами видалення тексту
#define MoveText 6 //Означає відображення текстового редактору з командами переміщення тексту
#define CopyText 7 //Означає відображення текстового редактору з командами копіювання тексту
#define AllSessions 8 //Означає відображення всіх сесій
#define Compare 9 //Означає порівняння з історії перед виконанням
#define FilesMenu 10 //Означає порівняння з історії перед виконанням

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

class ConcreteMemento : public Memento {
private:
    string SessionName;
    string SessionTextLine;
    string SessionTauntEvent;


public:
    ConcreteMemento(string Name, string TextLine, string TauntEvent) : SessionName(Name), SessionTextLine(TextLine), SessionTauntEvent(TauntEvent) {
    }

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

class Session {
private:
    string SessionName;
    string TextLine;
    string LastTauntEvent;
    bool Saved; //С этим ещё предстоит попариться
public:
    string GetName() {
        return SessionName;
    }

    Session(string SessionName, string TextLine = "", bool saved = false, string TauntEvent = "") : SessionName(SessionName), TextLine(TextLine), LastTauntEvent(TauntEvent), Saved(saved) {};

    string GetTextLine() {
        return TextLine;
    }

    bool GetSaved() {
        return Saved;
    }

    Memento* Save() {
        return new ConcreteMemento(SessionName, TextLine, LastTauntEvent);
    }

    void Restore(Memento* Backup) {
        SessionName = Backup->GetSessionName();
        TextLine = Backup->GetSessionTextLine();
        LastTauntEvent = Backup->GetSessionTauntEvent();
    }

    void SetTauntEvent(string NameOfTaunt) {
        LastTauntEvent = NameOfTaunt;
    }

    void InputToText(int Position, string Text) {
        TextLine.insert(Position, Text);
    }

    void DeleteFromText(int StartPos, int EndPos) {
        TextLine.erase(StartPos, EndPos);
    }

    void SaveFile(string NameSaveFile, bool Mode) {// true - перезапис, false - без перезапису
        Saved = true;
        fstream SaveStream(NameSaveFile, Mode ? ios::out : ios::app);
        SaveStream << SessionName << "+" << TextLine;
        SaveStream.close();
    }

};

class Caretaker {
private:
    vector<Memento*> mementos_;

    Session* originator_;

public:
    Caretaker(Session* originator) : originator_(originator) {
    }

    Caretaker(Caretaker& copy) {
        mementos_ = copy.mementos_;
        originator_ = copy.originator_;
    }

    ~Caretaker() {
        for (auto memento : mementos_) delete memento;
    }

    string GetUndoTextLine(int UndoNum) {
        return mementos_[UndoNum > mementos_.size()-1 ? mementos_.size()-1 : mementos_.size() - UndoNum]->GetSessionTextLine();
    }

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

    void MementosBackupFromFile(ConcreteMemento* memento) {
        mementos_.push_back(memento);
    }

    vector<Memento*> GetActiveSessionMemento() {
        return mementos_;
    }

    void SaveHistory(string NameSaveFile) {
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

    /*void ShowHistory() const {
        for (Memento* memento : this->mementos_) {
            cout << memento->GetSessionName() << "\n";
        }
    }*/
};

struct SavePattern {
    string SessionName;
    string TextLine;
};

struct SavePatterForMemento {
    string SessionName;
    string TextLine;
    string TauntEvent;
};

struct FunctorByLowest {
    bool operator()(pair<Session*, Caretaker*>& A, pair<Session*, Caretaker*>& B) {
        return B.first->GetName() > A.first->GetName();
    }
};

struct FunctorByHighest {
    bool operator()(pair<Session*, Caretaker*>& A, pair<Session*, Caretaker*>& B) {
        return A.first->GetName() > B.first->GetName();
    }
};

class Editor {
private:
    vector<pair<Session*, Caretaker*>> AvailableSessions;
    Session* ActiveSession;
    Caretaker* History;
    bool AlreadyLoaded;
    int UndoRewiewNumber;
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
        Render(FilesMenu);
        ActiveSession = NULL;
        AlreadyLoaded = false;
        UndoRewiewNumber = -1;
    }

    void ResetProgram() {
        ActiveSession = NULL;
        AlreadyLoaded = false;
        UndoRewiewNumber = -1;
        AvailableSessions.clear();
    }

    void SetNameSaveFile(string nameSaveFile) {
        NameSaveFile = nameSaveFile;
    }

    string GetNameSaveFile() { return NameSaveFile; }

    void SessionBackupFromSave() {
        fstream ReadStream(NameSaveFile, ios::in);

        if (!ReadStream.is_open()) {
            system("cls");
            cout << " !!! Збережень немає !!!" << endl;
            Render(MainMenu);
        }

        else if (AlreadyLoaded) {
            system("cls");
            cout << " !!! Ви вже завантажували дані !!!" << endl;
            Render(MainMenu);
        }

        string SaveLine;

        while (getline(ReadStream, SaveLine)) {
            SavePattern Example;
            istringstream AnotherStream(SaveLine);

            getline(AnotherStream, Example.SessionName, '+');
            getline(AnotherStream, Example.TextLine, '~');

            Session* TempSessionData = new Session(Example.SessionName, Example.TextLine, true);

            string MementoData;

            Caretaker* TempCaretaker = new Caretaker(TempSessionData);

            while (getline(AnotherStream, MementoData, '~')) {
                istringstream ParseMementoData(MementoData);

                SavePatterForMemento TempConcreteMemento;

                getline(ParseMementoData, TempConcreteMemento.SessionName, '+');
                getline(ParseMementoData, TempConcreteMemento.TextLine, '+');
                getline(ParseMementoData, TempConcreteMemento.TauntEvent, '~');

                TempCaretaker->MementosBackupFromFile(new ConcreteMemento(TempConcreteMemento.SessionName, TempConcreteMemento.TextLine, TempConcreteMemento.TauntEvent));
            }

            AvailableSessions.push_back(make_pair(TempSessionData, TempCaretaker));

        }

        system("cls");
        cout << " < Сеанси було завантажено >" << endl;

        AlreadyLoaded = true;

        Render(MainMenu);

        ReadStream.close();
    }

    void SessionsDelete() {
        int SessionIndex;

        cout << "Введіть індекс сеанс для видалення: ";
        cin >> SessionIndex;

        if (SessionIndex <= AvailableSessions.size() - 1 && !(SessionIndex < 0)) {
            delete AvailableSessions[SessionIndex].first;
            delete AvailableSessions[SessionIndex].second;

            AvailableSessions.erase(AvailableSessions.begin() + SessionIndex);
            system("cls");

            if (AvailableSessions.empty()) {
                Render(MainMenu);
            }
            else {
                SaveAllSessions();
                ShowAvailableSessionsForm();
            }
        }

        else {
            system("cls");
            cout << " !!! Ви обрали неіснуючий індекс !!!" << endl;
            ShowAvailableSessionsForm();
        }
    }

    void SaveAllSessionsForm() {
        system("cls");
        SaveAllSessions();
        cout << " < Дані було збережено у файл " << NameSaveFile << "!>" << endl;
        ShowAvailableSessionsForm();
    }

    void SortSessions(int SortIndex) {
        system("cls");

        if (SortIndex == SortByLowest) {
            sort(AvailableSessions.begin(), AvailableSessions.end(), FunctorByLowest());
        }
        else if (SortIndex == SortByHighest) {
            sort(AvailableSessions.begin(), AvailableSessions.end(), FunctorByHighest());
        }

        cout << " < Дані було відсортовано! >" << endl;
        ShowAvailableSessionsForm();
    }

    void SessionSelector() {
        int Choice;
        cin >> Choice;

        switch (Choice) {
        case 0: {
            system("cls");
            Render(MainMenu);
        }break;

        case 1: {
            SessionsDelete();
        }break;

        case 2: {
            SaveAllSessionsForm();
        }break;

        case 3: {
            SetActiveSessionForm();
        }break;

        case 4: {
            SortSessions(SortByHighest);
        }break;

        case 5: {
            SortSessions(SortByLowest);
        }break;

        default: {
            system("cls");
            cout << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(MainMenu);
        }
        }
    }

    void ShowAvailableSessionsForm() {
        if (!ShowAvailableSessions()) {
            cout << "!!! Немає доступних сесій !!!" << endl;
            Render(MainMenu);
            return;
        }
        else {
            cout << "1 - Видалити сеанс     2 - Зберегти всі сеанси у файл     3 - Обрати сеанс     \n4 - Сортувати за ростом     5 - Сортувати за убуванням     \n0 - Вихід" << endl << endl;
            cout << "Оберіть команду: ";
            SessionSelector();

            /*system("cls");
            Render(MainMenu);*/
        }
    }

    void SetActiveSessionForm() {
        cout << "Введіть номер сеансу: ";
        int SessionNumber;
        cin >> SessionNumber;

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

    void CreateNewSession() {

        string Name;
        cout << "Введіть ім'я сеансу: ";
        cin >> Name;

        string Text;
        cout << "Введіть початковий текст для сеансу (необов'язково): ";
        cin >> Text;

        system("cls");

        Session* session = new Session(Name, Text);
        Caretaker* caretaker = new Caretaker(session);

        AvailableSessions.push_back(make_pair(session, caretaker));
        SetActiveSession(AvailableSessions.size() - 1);

        Render(TextChange);
    }

    void NewSessionSelector() {
        int Choice;
        cin >> Choice;
        switch (Choice) {
        case 0: {
            system("cls");
            Render(FilesMenu);
        }break;

        case 1: {
            system("cls");
            CreateNewSession();
        }break;

        case 2: {
            system("cls");
            ShowAvailableSessionsForm();
        }break;

        case 3: {
            SessionBackupFromSave();
        }break;

        default: {
            system("cls");
            cout << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(MainMenu);
        }
        }
    }

    void InputToText() {
        int Choice;
        cin >> Choice;

        switch (Choice) {
        case 0: {
            system("cls");
            Render(TextChange);
        }break;

        case 1: {
            if (!OpenClipboard(nullptr)) {
                system("cls");
                cout << " !!! Не вдалось відкрити буфер обміну !!!" << endl;
                Render(TextChange);
                return;
            }

            HANDLE hData = GetClipboardData(CF_TEXT);
            if (hData == nullptr) {
                system("cls");
                cout << " !!! Не вдалось отримати дані з буферу обміну !!!" << endl;
                CloseClipboard();
                Render(TextChange);
                return;
            }

            char* buffer = static_cast<char*>(GlobalLock(hData));
            string clipboardText(buffer);
            GlobalUnlock(hData);

            CloseClipboard();

            int Pos;
            cout << " Введіть номер символу перед яким треба поставити текст: ";
            cin >> Pos;

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
            int Pos;
            cout << " Введіть номер символу перед яким треба поставити текст: ";
            cin >> Pos;

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
        cin >> Choice;

        switch (Choice) {
        case 0: {
            system("cls");
            Render(TextChange);
        }break;

        case 1: {
            int Pos, Count;
            cout << "Введіть через пробіл номер символу з якого почати вирізання та кількість символів після нього: ";
            cin >> Pos >> Count;
            string newText = ActiveSession->GetTextLine().substr(Pos, Count);

            if (!OpenClipboard(nullptr)) {
                system("cls");
                cout << " !!! Не вдалось відкрити буфер обміну !!!" << endl;
                Render(TextChange);
                return;
            }

            HANDLE hClipboardData = GetClipboardData(CF_TEXT);
            if (hClipboardData == nullptr) {
                system("cls");
                cout << " !!! Не вдалось отримати дані з буферу обміну !!!" << endl;
                CloseClipboard();
                Render(TextChange);
                return;
            }

            char* clipboardText = static_cast<char*>(GlobalLock(hClipboardData));

            string combinedText = clipboardText;
            combinedText += newText;

            GlobalUnlock(hClipboardData);

            HGLOBAL hNewClipboardData = GlobalAlloc(GMEM_MOVEABLE, combinedText.size() + 1);
            if (hNewClipboardData == nullptr) {
                system("cls");
                cout << " !!! Не вдалось виділити пам'ять для нових даних у буфері обміну !!!" << endl;
                CloseClipboard();
                Render(TextChange);
                return;
            }

            char* newBuffer = static_cast<char*>(GlobalLock(hNewClipboardData));
            strcpy_s(newBuffer, combinedText.size() + 1, combinedText.c_str());
            GlobalUnlock(hNewClipboardData);

            if (SetClipboardData(CF_TEXT, hNewClipboardData) == nullptr) {
                system("cls");
                cout << " !!! Не вдалось встановити нові дані у буфер обміну !!!" << endl;
                CloseClipboard();
                Render(TextChange);
                return;
            }

            CloseClipboard();

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
        cin >> Choice;

        switch (Choice) {
        case 0: {
            system("cls");
            Render(TextChange);
        }break;

        case 1: {
            int StartPos, Count;
            cout << "Введіть через пробіл номер символу з якого почати видалення та кількість символів після нього: ";
            cin >> StartPos >> Count;

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
        cin >> Choice;

        switch (Choice) {
        case 0: {
            system("cls");
            Render(TextChange);
        }break;

        case 1: {
            int StartPos, Count, Pos;
            cout << "Введіть через пробіл номер символу з якого почати виділення тексту та кількість символів після нього: ";
            cin >> StartPos >> Count;

            string SubText = ActiveSession->GetTextLine().substr(StartPos, Count);

            ActiveSession->SetTauntEvent("Переміщення");
            History->Backup();

            ActiveSession->DeleteFromText(StartPos, StartPos + Count + 1);

            system("cls");
            Render(SimpleTextChange);

            cout << " Введіть номер символу перед яким треба поставити текст: ";
            cin >> Pos;

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
        cin >> Choice;

        switch (Choice) {
        case 0: {
            system("cls");
            Render(TextChange);
        }break;

        case 1: {
            int Pos, Count;
            cout << "Введіть через пробіл номер символу з якого почати копіювання та кількість символів після нього: ";
            cin >> Pos >> Count;
            string newText = ActiveSession->GetTextLine().substr(Pos, Count);

            if (!OpenClipboard(nullptr)) {
                cout << " !!! Не вдалось відкрити буфер обміну !!!" << endl;
                Render(TextChange);
                return;
            }

            HANDLE hClipboardData = GetClipboardData(CF_TEXT);
            if (hClipboardData == nullptr) {
                cout << " !!! Не вдалось отримати дані з буферу обміну !!!" << endl;
                CloseClipboard();
                Render(TextChange);
                return;
            }

            char* clipboardText = static_cast<char*>(GlobalLock(hClipboardData));

            string combinedText = clipboardText;
            combinedText += newText;

            GlobalUnlock(hClipboardData);

            HGLOBAL hNewClipboardData = GlobalAlloc(GMEM_MOVEABLE, combinedText.size() + 1);
            if (hNewClipboardData == nullptr) {
                cout << " !!! Не вдалось виділити пам'ять для нових даних у буфері обміну !!!" << endl;
                CloseClipboard();
                Render(TextChange);
                return;
            }

            char* newBuffer = static_cast<char*>(GlobalLock(hNewClipboardData));
            strcpy_s(newBuffer, combinedText.size() + 1, combinedText.c_str());
            GlobalUnlock(hNewClipboardData);

            if (SetClipboardData(CF_TEXT, hNewClipboardData) == nullptr) {
                cout << " !!! Не вдалось встановити нові дані у буфер обміну !!!" << endl;
                CloseClipboard();
                Render(TextChange);
                return;
            }

            CloseClipboard();

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
        cin >> Choice;

        switch (Choice) {
        case 0: {
            system("cls");
            ActiveSession = NULL;
            Render(MainMenu);
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
        cin >> Choice;

        switch (Choice) {
        case 1: {
            History->Undo();
            Render(TextChange);
        }break;

        case 2: {
            int Number;
            cout << "Скільки подій ви бажаєте відмінити: ";
            cin >> Number;
            system("cls");
            UndoRewiewNumber = Number;
            Render(Compare);
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

    vector<pair<string,string>> getCOIFiles() {
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
        cin >> Choice;
        switch (Choice) {
        case 0: {
            exit(1);
        }break;
            
        case 1: {
            system("cls");
            CreateNewFile();
            system("cls");
            Render(MainMenu);
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
        cin >> Choice;
        switch (Choice) {
        case 0: {
            exit(1);
        }break;

        case 1: {
            system("cls");
            CreateNewFile();
            system("cls");
            Render(MainMenu);
        }break;

        case 2: {
            cout << " Оберіть індекс файлу: ";
            int Index;
            cin >> Index;
            ChooseTheFile(Index);
            system("cls");
            Render(MainMenu);
        }break;

        case 3: {
            cout << " Оберіть індекс файлу: ";
            int Index;
            cin >> Index;
            
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

    void Render(int Variative) {
        if (Variative == FilesMenu) {
            vector<pair<string, string>> Paths = getCOIFiles();
            if (!Paths.empty()) {
                cout << "Список доступних файлів:" << endl;
                cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;

                for (int i = 0; i < Paths.size(); i++) {
                    cout << "| " << setw(4) << setprecision(4) << setfill(' ') << left << i << setw(112) << setprecision(112) << setfill(' ') << Paths[i].first.substr(0, 111) << "|" << endl;
                }

                cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;
                cout << "1 - Створити файл      2 - Обрати файл    3 - Видалити файл     0 - Вихід з програми" << endl;
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
                cout << "1 - Створити файл      0 - Вихід з програми" << endl;
                cout << endl << " Оберіть команду: ";
                NewFileSelector();
            }
        }
        if (Variative == MainMenu) {
            cout << "Ім'я файлу: " << GetNameSaveFile() << endl;
            cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|                                             Створіть або оберіть сесію!                                             |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;
            cout << "1 - Створити сеанс      2 - Перегляд сеансів та дії з ними      3 - Завантажити сеанси з файлу     0 - Обзор файлів" << endl;
            cout << endl << " Оберіть команду: ";
            NewSessionSelector();
        }

        else if (Variative >= TextChange && Variative <= CopyText) {

            vector<Memento*> TempMemento = History->GetActiveSessionMemento();

            int CountCoef = TempMemento.size() > 5 ? TempMemento.size() - 5 : 0;


            cout << "[Ім'я сесії: " << ActiveSession->GetName() << "]" << endl;
            cout << " ----------------------------------------------------------------------------------------------------" << "------------------" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 5 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 5 ? (*(TempMemento.begin() + 4 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 4 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 4 ? (*(TempMemento.begin() + 3 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|" << setw(100) << setprecision(100) << setfill(' ') << left << ActiveSession->GetTextLine().substr(0, 99) << "|" << setw(3) << setprecision(3) << setfill(' ') << left << 3 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 3 ? (*(TempMemento.begin() + 2 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 2 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 2 ? (*(TempMemento.begin() + 1 + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << 1 + CountCoef << setw(14) << setprecision(14) << setfill(' ') << left << (TempMemento.size() >= 1 ? (*(TempMemento.begin() + CountCoef))->GetSessionTauntEvent() : "Запису немає") << "|" << endl;
            cout << " ----------------------------------------------------------------------------------------------------" << "------------------" << endl;
            switch (Variative) {
            case TextChange: {
                cout << "1 - Вставити      2 - Вирізати      3 - Видалити     4 - Перемістити     5 - Скопіювати     6 - Відмінити     \n0 - Вихід" << endl;
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
        else if (Variative == Compare) {
            
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
            cout << "|" << setw(100) << setprecision(100) << setfill(' ') << left << History->GetUndoTextLine(UndoRewiewNumber).substr(0, 99) << "                 |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << " ----------------------------------------------------------------------------------------------------------------------" << endl;
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
