#ifndef TOKEN_SET_H
#define TOKEN_SET_H

TokenSet* createTokenSet();
void destroyTokenSet(TokenSet* ts);
void addToken(TokenSet* ts, const char* tok);
void removeToken(TokenSet* ts, const char* tok);
int isMember(TokenSet* ts, const char* tok);

#endif