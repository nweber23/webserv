#pragma once

#include <map>
#include <string>
#include <ctime>
#include <pthread.h>

struct SessionData
{
	unsigned long counter;
	time_t lastSeen;

	SessionData() : counter(0), lastSeen(0) {}
};

class SessionStore
{
private:
	std::map<std::string, SessionData> _sessions;
	pthread_mutex_t _mtx;
	unsigned int _ttlSeconds;

	SessionStore();
	SessionStore(const SessionStore& other);
	SessionStore& operator=(const SessionStore& other);

	void cleanupExpiredLocked(time_t now);

public:
	~SessionStore();

	static SessionStore& instance();

	SessionData getOrCreate(const std::string& sid, bool& created);
	void put(const std::string& sid, const SessionData& data);
	bool exists(const std::string& sid);
	void setTtlSeconds(unsigned int ttlSeconds);
};
