#include <sql_connection_pool.h>
#include <../log/log.h>

ConnectionPool::ConnectionPool() {
    this->m_cur_conn = 0;
    this->m_free_conn = 0;
}

ConnectionPool::~ConnectionPool() {
    sem_destroy(&this->m_sem_t);
    this->destroyPool();
}

ConnectionPool* ConnectionPool::getInstance() {
    static ConnectionPool instance;
    return &instance;
}

void ConnectionPool::init(std::string url, std::string port, std::string user, std::string pwd, std::string db_name, int max_conn, bool close_log) {
    this->m_url = url;
    this->m_port = port;
    this->m_user = user;
    this->m_pwd = pwd;
    this->m_db_name = db_name;
    this->m_close_log = close_log;

    for (size_t i = 0; i < max_conn; ++i) {
        MYSQL* conn = NULL;
        conn = mysql_init(conn);
        if (conn == NULL) {
            LOG_ERROR("mysql init error");
        }
        if (!mysql_real_connect(conn, url.c_str(), user.c_str(), pwd.c_str(), db_name.c_str(), atoi(port.c_str()), NULL, 0)) {
            LOG_ERROR("mysql connect error");
        }
        this->m_conn_list.push_back(conn);
        ++this->m_free_conn;
    }
    sem_init(&this->m_sem_t, 0, max_conn);
    this->m_max_conn = this->m_free_conn;
}

MYSQL* ConnectionPool::getConnection() {
    if (this->m_free_conn == 0) {
        return NULL;
    }

    sem_wait(&this->m_sem_t);

    MYSQL* result = NULL;
    this->m_mutex.lock();

    result = this->m_conn_list.front();
    this->m_conn_list.pop_front();
    --this->m_free_conn;
    ++this->m_cur_conn;

    this->m_mutex.unlock();
    return result;
}

int ConnectionPool::getFreeConnNum() {
    int temp = 0;
    this->m_mutex.lock();
    temp = this->m_free_conn;
    this->m_mutex.unlock();
    return temp;
}

bool ConnectionPool::releaseConnection(MYSQL* conn) {
    if (conn == NULL)
        return false;

    this->m_mutex.lock();

    this->m_conn_list.push_back(conn);
    ++this->m_free_conn;
    --this->m_cur_conn;

    this->m_mutex.unlock();

    sem_post(&this->m_sem_t);
    return true;
}

void ConnectionPool::destroyPool() {
    this->m_mutex.lock();

    for (auto iter = this->m_conn_list.begin(); iter != this->m_conn_list.end(); ++iter) {
        mysql_close(*iter);
    }

    this->m_free_conn = 0;
    this->m_cur_conn = 0;

    this->m_mutex.unlock();
}

ConnectionPollRAII::ConnectionPollRAII(MYSQL** sql, ConnectionPool* conn) {
    *sql = conn->getConnection();
    this->m_sql = *sql;
    this->m_conn_pool = conn;
}

ConnectionPollRAII::~ConnectionPollRAII() {
    this->m_conn_pool->releaseConnection(this->m_sql);
}