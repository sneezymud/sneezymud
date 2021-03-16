#include <cxxtest/TestSuite.h>

#include "connect.h"
#include "socket.h"
#include "DescriptorList.h"

extern Descriptor *descriptor_list;

class DescriptorListTest : public CxxTest::TestSuite
{
 public:

  void testLoopEmpty()
  {
    descriptor_list = nullptr;

    for (Descriptor* _ : DescriptorList) {
      (void)_;
      TS_ASSERT(false); // unreachable code due to 0 loop steps
    }
  }

  void testIncrement()
  {
    descriptor_list = new Descriptor(new TSocket());

    auto it = DescriptorList.begin();
    ++it;
    TS_ASSERT_EQUALS(nullptr, *it);

    delete descriptor_list;
  }

  void testLoopOne()
  {
    descriptor_list = new Descriptor(new TSocket());
    descriptor_list->socket->m_sock = 1234;

    int count = 0;
    for (Descriptor* d : DescriptorList)
    {
      ++count;
      TS_ASSERT_EQUALS(1234, d->socket->m_sock);
    }

    TS_ASSERT_EQUALS(1, count);

    delete descriptor_list;
  }

  void testLoopTwo()
  {
    descriptor_list = new Descriptor(new TSocket());
    descriptor_list->socket->m_sock = 2222;
    Descriptor* tmp = new Descriptor(new TSocket());
    tmp->socket->m_sock = 1111;
    // they're chained into a linked list automatically in constructor

    int count = 0;
    for (Descriptor* d : DescriptorList)
    {
      ++count;
      TS_ASSERT_EQUALS(count == 1 ? 1111 : 2222, d->socket->m_sock);
    }

    TS_ASSERT_EQUALS(2, count);

    delete descriptor_list;
  }
};
