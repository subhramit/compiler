#include "TokenSet.c"

int main() {
    TokenSet* ts = createTokenSet();
    if (!ts) {
        printf("Failed to create token set\n");
        return EXIT_FAILURE;
    }
    addToken(ts, "token1");
    addToken(ts, "token2");
    addToken(ts, "token3");

    printf("Is 'token2' in set? %s\n", isMember(ts, "token2") ? "Yes" : "No");
    printf("Is 'token4' in set? %s\n", isMember(ts, "token4") ? "Yes" : "No");

    removeToken(ts, "token2");

    printf("Is 'token2' in set after removal? %s\n", isMember(ts, "token2") ? "Yes" : "No");

    destroyTokenSet(ts);
    return EXIT_SUCCESS;
}