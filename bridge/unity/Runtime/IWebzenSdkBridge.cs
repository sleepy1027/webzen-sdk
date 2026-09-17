namespace Webzen
{
    /// <summary>
    /// One implementation per platform (Android/iOS/Windows), each a thin
    /// wrapper over that platform's native artifact. WebzenSDK picks the
    /// right one at static init time; no business logic belongs here or in
    /// any implementation -- see the architecture skill in the main repo
    /// (.claude/skills/game-sdk-core-architecture) for why.
    /// </summary>
    internal interface IWebzenSdkBridge
    {
        void Initialize(string baseUrl);
        void Login(string providerId, string providerToken, WebzenSDK.LoginCallback callback);
    }
}
