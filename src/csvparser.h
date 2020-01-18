
#include <QList>
#include <QByteArray>

namespace osmin
{
  class CSVParser
  {
  public:
    CSVParser(const char separator, const char encapsulator);
    virtual ~CSVParser() { }

    QList<QByteArray*> deserialize(const QByteArray& line);
    QByteArray serialize(const QList<QByteArray*> row);

  private:
    char m_separator;
    char m_encapsulator;
  };
}
