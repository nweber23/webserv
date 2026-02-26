#include "SessionStore.hpp"

#include <cstdlib>

SessionStore::SessionStore() : _sessions(), _ttlSeconds(60 * 60)
{
	pthread_mutex_init(&_mtx, NULL);
}

SessionStore::SessionStore(const SessionStore& other) : _sessions(other._sessions), _ttlSeconds(other._ttlSeconds)
{
	pthread_mutex_init(&_mtx, NULL);
}

SessionStore& SessionStore::operator=(const SessionStore& other)
{
	if (this != &other)
	{
		pthread_mutex_lock(&_mtx);
		_sessions = other._sessions;
		_ttlSeconds = other._ttlSeconds;
		pthread_mutex_unlock(&_mtx);
	}
	return *this;
}

SessionStore::~SessionStore()
{
	pthread_mutex_destroy(&_mtx);
}

SessionStore& SessionStore::instance()
{
	static SessionStore inst;
	return inst;
}

void SessionStore::setTtlSeconds(unsigned int ttlSeconds)
{
	pthread_mutex_lock(&_mtx);
	_ttlSeconds = ttlSeconds;
	pthread_mutex_unlock(&_mtx);
}

void SessionStore::cleanupExpiredLocked(time_t now)
{
	for (std::map<std::string, SessionData>::iterator it = _sessions.begin(); it != _sessions.end(); )
	{
		if (it->second.lastSeen != 0 && (unsigned int)(now - it->second.lastSeen) > _ttlSeconds)
			it = _sessions.erase(it);
		else
			++it;
	}
}

bool SessionStore::exists(const std::string& sid)
{
	pthread_mutex_lock(&_mtx);
	bool ok = _sessions.find(sid) != _sessions.end();
	pthread_mutex_unlock(&_mtx);
	return ok;
}

SessionData SessionStore::getOrCreate(const std::string& sid, bool& created)
{
	created = false;
	time_t now = std::time(NULL);
	SessionData data;

	pthread_mutex_lock(&_mtx);
	cleanupExpiredLocked(now);
	std::map<std::string, SessionData>::iterator it = _sessions.find(sid);
	if (it == _sessions.end())
	{
		created = true;
		data.counter = 0;
		data.lastSeen = now;
		_sessions[sid] = data;
	}
	else
	{
		data = it->second;
		data.lastSeen = now;
		it->second.lastSeen = now;
	}
	pthread_mutex_unlock(&_mtx);
	return data;
}

void SessionStore::put(const std::string& sid, const SessionData& data)
{
	pthread_mutex_lock(&_mtx);
	_sessions[sid] = data;
	pthread_mutex_unlock(&_mtx);
}
