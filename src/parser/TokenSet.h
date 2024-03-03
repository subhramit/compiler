TokenSet* createTokenSet();
void destroyTokenSet(TokenSet* ts);
void addToken(TokenSet* ts, const char* tok);
void removeToken(TokenSet* ts, const char* tok);
int isMember(TokenSet* ts, const char* tok);