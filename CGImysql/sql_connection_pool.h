#ifndef SQL_CONNECTION_POOL_H_
#define SQL_CONNECTION_POOL_H_

#include <mysql/mysql.h>
#include <semaphore.h>
#include <list>
#include <../utils/locker.h>

class ConnectionPool {
public:
    MYSQL* getConnection();
    bool releaseConnection(MYSQL*);
    int getFreeConnNum();

    void init(std::string url, std::string port, std::string user, std::string pwd, std::string db_name, int max_conn, bool close_log);

    static ConnectionPool* getInstance();

private:
    ConnectionPool();
    ~ConnectionPool();
    void destroyPool();

    int m_max_conn;
    int m_free_conn;
    int m_cur_conn;

    locker m_mutex;
    sem_t m_sem_t;
    std::list<MYSQL*> m_conn_list;

public:
    std::string m_url;
    std::string m_port;
    std::string m_user;
    std::string m_pwd;
    std::string m_db_name;
    bool m_close_log;
};

class ConnectionPollRAII {
public:
    ConnectionPollRAII(MYSQL**, ConnectionPool*);
    ~ConnectionPollRAII();
public:
    MYSQL* m_sql;
    ConnectionPool* m_conn_pool;

};

#endif