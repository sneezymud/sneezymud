
#ifndef __MAIN_H
#define __MAIN_H

class CMainApp {
  // construction
  public:
    CMainApp(int &argc, const char & argv[]);
    ~CMainApp();

  // disallow these funcs
  private:
    CMainApp(const CMainApp &) {}
    CMainApp & operator = (const CMainApp &) {}
    CMainApp() {}

  // accessors
  public:
    inline const unsigned int & GetPort() const
    {
      return m_port;
    }
    inline void SetPort(const unsigned int & port)
    {
      m_port = port;
    }
 
  // member variables
  private:
    unsigned int m_port;
};

#endif
