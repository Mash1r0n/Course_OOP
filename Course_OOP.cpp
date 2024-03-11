#include <iostream>
#include <vector>
using namespace std;

class Memento {
public:
    virtual ~Memento() {}
    virtual std::string GetName() const = 0;
    virtual std::string date() const = 0;
    virtual std::string state() const = 0;
};

/**
 * The Concrete Memento contains the infrastructure for storing the Originator's
 * state.
 */
class ConcreteMemento : public Memento {
private:
    std::string state_;
    std::string date_;

public:
    ConcreteMemento(std::string state) : state_(state) {
        this->state_ = state;
        std::time_t now = std::time(0);
    }
    /**
     * The Originator uses this method when restoring its state.
     */
    std::string state() const override {
        return this->state_;
    }
    /**
     * The rest of the methods are used by the Caretaker to display metadata.
     */
    std::string GetName() const override {
        return this->date_ + " / (" + this->state_.substr(0, 9) + "...)";
    }
    std::string date() const override {
        return this->date_;
    }
};

/**
 * The Originator holds some important state that may change over time. It also
 * defines a method for saving the state inside a memento and another method for
 * restoring the state from it.
 */
class Originator {
    /**
     * @var string For the sake of simplicity, the originator's state is stored
     * inside a single variable.
     */
private:
    std::string state_;

    std::string GenerateRandomString(int length = 10) {
        const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        int stringLength = sizeof(alphanum) - 1;

        std::string random_string;
        for (int i = 0; i < length; i++) {
            random_string += alphanum[std::rand() % stringLength];
        }
        return random_string;
    }

public:
    Originator(std::string state) : state_(state) {
        std::cout << "Originator: My initial state is: " << this->state_ << "\n";
    }
    /**
     * The Originator's business logic may affect its internal state. Therefore,
     * the client should backup the state before launching methods of the business
     * logic via the save() method.
     */
    void DoSomething() {
        std::cout << "Originator: I'm doing something important.\n";
        this->state_ = this->GenerateRandomString(30);
        std::cout << "Originator: and my state has changed to: " << this->state_ << "\n";
    }

    /**
     * Saves the current state inside a memento.
     */
    Memento* Save() {
        return new ConcreteMemento(this->state_);
    }
    /**
     * Restores the Originator's state from a memento object.
     */
    void Restore(Memento* memento) {
        this->state_ = memento->state();
        std::cout << "Originator: My state has changed to: " << this->state_ << "\n";
    }
};

/**
 * The Caretaker doesn't depend on the Concrete Memento class. Therefore, it
 * doesn't have access to the originator's state, stored inside the memento. It
 * works with all mementos via the base Memento interface.
 */
class Caretaker {
    /**
     * @var Memento[]
     */
private:
    std::vector<Memento*> mementos_;

    /**
     * @var Originator
     */
    Originator* originator_;

public:
    Caretaker(Originator* originator) : originator_(originator) {
    }

    ~Caretaker() {
        for (auto m : mementos_) delete m;
    }

    void Backup() {
        std::cout << "\nCaretaker: Saving Originator's state...\n";
        this->mementos_.push_back(this->originator_->Save());
    }
    void Undo() {
        if (!this->mementos_.size()) {
            return;
        }
        Memento* memento = this->mementos_.back();
        this->mementos_.pop_back();
        std::cout << "Caretaker: Restoring state to: " << memento->GetName() << "\n";
        try {
            this->originator_->Restore(memento);
        }
        catch (...) {
            this->Undo();
        }
    }
    void ShowHistory() const {
        std::cout << "Caretaker: Here's the list of mementos:\n";
        for (Memento* memento : this->mementos_) {
            std::cout << memento->GetName() << "\n";
        }
    }
};
/**
 * Client code.
 */

void ClientCode() {
    Originator* originator = new Originator("Super-duper-super-puper-super.");
    Caretaker* caretaker = new Caretaker(originator);
    caretaker->Backup();
    originator->DoSomething();
    caretaker->Backup();
    originator->DoSomething();
    caretaker->Backup();
    originator->DoSomething();
    std::cout << "\n";
    caretaker->ShowHistory();
    std::cout << "\nClient: Now, let's rollback!\n\n";
    caretaker->Undo();
    std::cout << "\nClient: Once more!\n\n";
    caretaker->Undo();

    delete originator;
    delete caretaker;
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(NULL)));
    ClientCode();
    return 0;
}








#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <iomanip>
using namespace std;

#define MainMenu 0 //Означає відображенняж. головного меню
#define TextChange 1 //Означає відображення текстового редактору
#define InputText 2 //Означає відображення текстового редактору з командами вставки тексту
#define CutText 3 //Означає відображення текстового редактору з командами вирізки тексту
#define DeleteText 4 //Означає відображення текстового редактору з командами видалення тексту
#define MoveText 5 //Означає відображення текстового редактору з командами переміщення тексту
#define CopyText 6 //Означає відображення текстового редактору з командами копіювання тексту
#define AllSessions 7 //Означає відображення всіх сесій

class Session {
private:
    string SessionName;
    string TextLine;
public:
    string GetName() {
        return SessionName;
    }

    Session(string SessionName, string TextLine = "") : SessionName(SessionName), TextLine(TextLine) {};

    string GetTextLine() {
        return TextLine;
    }

    void InputToText(int Position, string Text) {
        TextLine.insert(Position, Text);
    }

    void DeleteFromText(int StartPos, int EndPos) {
        TextLine.erase(StartPos, EndPos);
    }
};

class Editor {
private:
    vector<Session> AvailableSessions;
    Session* ActiveSession;

    bool ShowAvailableSessions() {
        if (AvailableSessions.empty()) {
            return false;
        }
        Render(AllSessions);
        return true;
    }

    bool SetActiveSession(int SessionIndex) {
        if (SessionIndex <= AvailableSessions.size() - 1 && !(SessionIndex < 0)) {
            ActiveSession = &AvailableSessions[SessionIndex];
            return true;
        }
        else {
            return false;
        }
    }
public:
    Editor() {
        Render(MainMenu);
        ActiveSession = NULL;
    }

    void ShowAvailableSessionsForm() {
        system("cls");
        if (!ShowAvailableSessions()) {
            cout << "!!! Немає доступних сесій !!!" << endl;
            Render(MainMenu);
            return;
        }
        else {
            system("pause");
            system("cls");
            Render(MainMenu);
        }
    }

    void SetActiveSessionForm() {
        system("cls");
        ShowAvailableSessions();

        if (AvailableSessions.size() == 0) {
            cout << "!!! Немає жодної сесії !!!" << endl;
            Render(MainMenu);
        }

        cout << "Введіть номер сесії: ";
        int SessionNumber;
        cin >> SessionNumber;

        if (SetActiveSession(SessionNumber)) {
            system("cls");
            Render(TextChange);
        }

        else {
            system("cls");
            cout << "!!! Сесії під таким індексом не існує !!!" << endl;
            Render(MainMenu);
            return;
        }
    }

    void CreateNewSession() {
        system("cls");

        string Name;
        cout << "Введіть ім'я сесії: ";
        cin >> Name;

        string Text;
        cout << "Введіть початковий текст для сесії (необов'язково): ";
        cin.ignore();
        getline(std::cin, Text);

        system("cls");

        AvailableSessions.push_back(Session(Name, Text));
        SetActiveSession(AvailableSessions.size() - 1);

        Render(TextChange);
    }

    void NewSessionSelector() {
        int Choice;
        cin >> Choice;
        switch (Choice) {
        case 0: {
            exit;
        }break;

        case 1: {
            CreateNewSession();
        }break;

        case 2: {
            ShowAvailableSessionsForm();
        }break;

        case 3: {
            //Файл!
        }break;

        case 4: {
            SetActiveSessionForm();
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

            ActiveSession->InputToText(Pos, clipboardText);

            Render(TextChange);
        }break;

        case 2: {
            string Text;
            cout << " Введіть текст, який треба вставити: ";
            cin.ignore();
            getline(cin, Text);
            int Pos;
            cout << " Введіть номер символу перед яким треба поставити текст: ";
            cin >> Pos;

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

        }break;

        case 5: {

        }break;
        default: {
            system("cls");
            cout << right << " !!! Ви обрали неіснуючу дію !!!" << endl;
            Render(TextChange);
        }
        }
    }

    void Render(int Variative) {
        if (Variative == MainMenu) {
            cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|                                             Створіть або оберіть сесію!                                             |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << "|                                                                                                                     |" << endl;
            cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;
            cout << "1 - Створити сесію      2 - Подивитися перелік сесій      3 - Завантажити сесії з файлу     4 - Обрати сесію     \n0 - Вихід з програми" << endl;
            cout << endl << " Оберіть команду: ";
            NewSessionSelector();
        }

        else if (Variative >= TextChange && Variative <= CopyText) {
            cout << "[Ім'я сесії: " << ActiveSession->GetName() << "]" << endl;
            cout << " ----------------------------------------------------------------------------------------------------" << "------------------" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << "3" << setw(14) << setprecision(14) << setfill(' ') << left << "AsssssssAA" << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << "3" << setw(14) << setprecision(14) << setfill(' ') << left << "AAsadasdA" << "|" << endl;
            cout << "|" << setw(100) << setprecision(100) << setfill(' ') << left << ActiveSession->GetTextLine().substr(0, 99) << "|" << setw(3) << setprecision(3) << setfill(' ') << left << "3" << setw(14) << setprecision(14) << setfill(' ') << left << "AasdasdAA" << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << "3" << setw(14) << setprecision(14) << setfill(' ') << left << "AAA" << "|" << endl;
            cout << "|                                                                                                    |" << setw(3) << setprecision(3) << setfill(' ') << left << "3" << setw(14) << setprecision(14) << setfill(' ') << left << "AAasdA" << "|" << endl;
            cout << " ----------------------------------------------------------------------------------------------------" << "------------------" << endl;
            switch (Variative) {
            case TextChange: {
                cout << "1 - Вставити      2 - Вирізати      3 - Видалити     4 - Перемістити     5 - Скопіювати     6 - Відмінити     \n7 - Зберегти у файл     0 - Вихід" << endl;
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
                //Переміщення тексту
            }break;

            case CopyText: {
                //Копіювання тексту
            }break;
            }

        }
        else if (Variative == AllSessions) {
            cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;

            for (int i = 0; i < AvailableSessions.size(); i++) {
                cout << "| " << setw(4) << setprecision(4) << setfill(' ') << left << i << setw(112) << setprecision(112) << setfill(' ') << AvailableSessions[i].GetName().substr(0, 111) << "|" << endl;
            }

            cout << " ---------------------------------------------------------------------------------------------------------------------" << endl;
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
