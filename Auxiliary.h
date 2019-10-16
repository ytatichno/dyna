#pragma once

#include <string>
#include <list>
#include <vector>

///////////////////////////////////
//// Вспомогательные функции  ////
/////////////////////////////////
namespace Auxiliary
{

  // Функция переводит элемент типа Т в std::string (в качестве типа может быть число)
  template <typename T>
  std::string ToString(T value);

  // Функция переводит из std::string в элемент типа T
  template <typename T>
  T FromString(const std::string& s);

  std::string BoolToString(bool value);

  // Функция выводит строку звездочек
  void StarsStringToFlow(std::ofstream& outFile);

  //**************************************************************************************//
  //**************************************************************************************//
  //**************************************************************************************//
  // Класс, делящий текст из файла, на вектора слов (каждая строчка - вектор)
  //**************************************************************************************//
  //**************************************************************************************//
  //**************************************************************************************//

  // ПРИМЕР:
  // Создаем хранилище строк
  //    SeparatorFlowToTheWords* store;
  // Набор разделителей, переводов строк и пустых символов
  //     store->AddEmptySymbol('\t');
  //     store->AddEmptySymbol(' ');
  //     store->AddEmptySymbol('\b');
  //     store->AddEmptySymbol('\v');
  //     store->AddEndLinesSymbol('\r');
  //
  //     store->AddEndLinesSymbol('\n');
  //
  //     store->AddDivider('*');
  //     store->AddDivider('=');
  //     store->AddDivider(';');
  // Фиксируем необходимость рассматривать концы строк в качестве слов (По умолчанию они сбрасываются)
  //    store->SetIsFixEndLinesSymbols();
  // Фиксируем необходимость рассматривать разделители в качестве слов (По умолчанию они сбрасываются)
  //    store->SetIsFixDividers();
  // Заполняем хранилище строк по данным из файла

  // Вспомогательная сущность:
  //строка текста из файла (список слов, без разделителей, указанных в классе SeparatorFlowToTheWords)
#define  splitString std::vector<std::string>*
  //

  class SeparatorFlowToTheWords{

    std::vector<splitString>  m_splitStringsTable;  // все строчки текста (вектор списков)

    bool            m_isFixDividers;    // фиксировать разделители в отдельные слова строчки
                              // по умолчанию - ложь

    bool            m_isFixEndLinesSyms;  // Фиксировать символы конца строки в отдельные слова
                              // строчки, по умолчанию - ложь

    std::list<char>        m_dividers;        // разделители слов в строчке (необходимо задать при инициализации)
    std::list<char>        m_emptySymbols;      // пустые символы в строчке (необходимо задать при инициализации)
    std::list<char>        m_endLinesSymbols;    // символы конца строк (необходимо задать при инициализации)

    int              m_errID;        // Идентификатор ошибки, возникшей при работе с объектом класса
    // -1  - все впорядке
    //  10  - не удалось открыть файл для обработки
  public:

    // Конструктор
    SeparatorFlowToTheWords();
    // Деструктор
    ~SeparatorFlowToTheWords();

  public:
    // Возвращает все строчки файла
    std::vector<splitString>*  GetStringsStore();
    // Возвращаетстрочку
    splitString          GetString(unsigned i);
    // Возвращает размер хранилища
    unsigned          GetSizeOfStore();
    // Возвращает размер хранилища
    bool            IsEmptyStore();
    // Имели место ошибки
    bool            AreThereErrors();
    // Возвращает идентификатор ошибки
    int              GetErrorID();
  public:
    // Переводит данные файла в хранилище строк
    void TranslatesFilesToStringsStore(const char* fileName);
    // Устанавливает необходимость фиксирования разделителей
    void SetIsFixDividers(bool flag = true );
    // Устанавливает необходимость фиксирования символов конца строки
    void SetIsFixEndLinesSymbols(bool flag = true );
    // Добавляет символов разделитель
    void AddDivider(char c);
    // Добавляет пустой символ
    void AddEmptySymbol(char c);
    // Добавляет символ конца строки
    void AddEndLinesSymbol(char c);
    // Проверяет, является ли символ разделителем
    bool IsDivider(char c);
    // Проверяет, является ли символ пустым
    bool IsEmptySymbol(char c);
    // Проверяет, является ли символ концом строки
    bool IsEndLinesSymbol(char c);

  private:

    std::string          m_currStr;        // Текущая незаконченная строка
    splitString          m_currSplitStr;      // Текущая незаконченная строка таблицы (по умолчанию - NULL)

    // Добавляет строку к m_currSplitStr
    void AddString();
    // Добавляет SplitStr к вектору SplitStr-ов
    void AddSplitSring();
  };
}
///////////////////////////