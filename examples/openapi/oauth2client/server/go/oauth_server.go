// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// A deliberately small OAuth 2.0 authorization server for the Qt OpenAPI
// oauth2client example. It implements the authorization code grant with
// mandatory PKCE (RFC 6749, RFC 7636) and refresh tokens, keeps everything in
// memory, and knows exactly one client and one demo user. It is not meant to
// be used for anything but the example.

package openapi

import (
	"crypto/rand"
	"crypto/sha256"
	"encoding/base64"
	"html/template"
	"net/http"
	"net/url"
	"slices"
	"strings"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
)

// ClientID is the one client this server accepts. Client IDs of native
// applications are public by design (RFC 8252, section 8.5), so the example
// application simply hardcodes it.
const ClientID = "qt-oauth2-example"

const (
	authorizationCodeLifetime = time.Minute
	// Short on purpose, so that the automatic token refresh of the example is
	// easy to observe.
	accessTokenLifetime = 30 * time.Second
)

var knownScopes = map[string]string{
	"profile:read": "Read your display name and your country",
	"email:read":   "Read your email address",
}

// Session is what an access token or a refresh token stands for.
type Session struct {
	Scope []string
	User  UserProfile
}

func (s *Session) HasScope(scope string) bool {
	return slices.Contains(s.Scope, scope)
}

type pendingCode struct {
	Session
	redirectURI   string
	codeChallenge string
	expiresAt     time.Time
}

type accessToken struct {
	Session
	expiresAt time.Time
}

type AuthorizationServer struct {
	mu            sync.Mutex
	codes         map[string]pendingCode
	accessTokens  map[string]accessToken
	refreshTokens map[string]Session
}

func NewAuthorizationServer() *AuthorizationServer {
	return &AuthorizationServer{
		codes:         make(map[string]pendingCode),
		accessTokens:  make(map[string]accessToken),
		refreshTokens: make(map[string]Session),
	}
}

// RegisterRoutes adds the OAuth 2.0 endpoints, which are not part of the
// OpenAPI specification, to the router generated from it.
func (s *AuthorizationServer) RegisterRoutes(router *gin.Engine) {
	router.GET("/oauth/authorize", s.showConsentPage)
	router.POST("/oauth/authorize", s.handleConsent)
	router.POST("/oauth/token", s.issueToken)
}

// Authenticate checks the Bearer token of a resource request. On failure it
// writes the error response as described in RFC 6750 and returns false.
func (s *AuthorizationServer) Authenticate(c *gin.Context,
	requiredScope string) (*Session, bool) {
	const realm = `Bearer realm="qt-oauth2-example"`
	token, found := strings.CutPrefix(c.GetHeader("Authorization"), "Bearer ")
	if !found || token == "" {
		c.Header("WWW-Authenticate", realm)
		c.AbortWithStatus(http.StatusUnauthorized)
		return nil, false
	}

	s.mu.Lock()
	access, ok := s.accessTokens[token]
	s.mu.Unlock()

	if !ok || time.Now().After(access.expiresAt) {
		c.Header("WWW-Authenticate", realm+`, error="invalid_token"`)
		c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{
			"error":             "invalid_token",
			"error_description": "The access token is unknown or has expired",
		})
		return nil, false
	}
	if !access.HasScope(requiredScope) {
		c.Header("WWW-Authenticate", realm+`, error="insufficient_scope", scope="`+requiredScope+`"`)
		c.AbortWithStatusJSON(http.StatusForbidden, gin.H{
			"error":             "insufficient_scope",
			"error_description": "The access token does not cover " + requiredScope,
		})
		return nil, false
	}
	return &access.Session, true
}

// authorizationRequest holds the validated parameters of an authorization
// request (RFC 6749, section 4.1.1), from the query of the initial GET as well
// as from the form fields of the consent page.
type authorizationRequest struct {
	redirectURI   *url.URL
	state         string
	scope         []string
	codeChallenge string
}

// parseAuthorizationRequest validates the request. It returns an RFC 6749
// error code and description if the request must be rejected. Only errors
// that leave the redirect URI trustworthy are reported to that URI.
func parseAuthorizationRequest(params url.Values) (req authorizationRequest,
	errCode, errDescription string) {
	if params.Get("client_id") != ClientID {
	    // Checked first: an unknown client must not be redirected to
		// (RFC 6749, section 4.1.2.1).
		return req, "unauthorized_client",
		    "unknown client_id, the demo server only knows " + ClientID
	}
	redirectURI, err := url.Parse(params.Get("redirect_uri"))
	if err != nil || redirectURI.Scheme != "http" || !isLoopbackHost(redirectURI.Hostname()) {
	    return req, "invalid_request", "redirect_uri must be an http URL on the loopback interface"
	}
	req.redirectURI = redirectURI
	req.state = params.Get("state")
	if params.Get("response_type") != "code" {
		return req, "unsupported_response_type", "only response_type=code is supported"
	}
	req.codeChallenge = params.Get("code_challenge")
	if req.codeChallenge == "" || params.Get("code_challenge_method") != "S256" {
		return req, "invalid_request", "PKCE with code_challenge_method=S256 is required"
	}
	req.scope = strings.Fields(params.Get("scope"))
	for _, scope := range req.scope {
		if _, ok := knownScopes[scope]; !ok {
			return req, "invalid_scope", "unknown scope " + scope
		}
	}
	return req, "", ""
}

func isLoopbackHost(host string) bool {
	return host == "127.0.0.1" || host == "::1" || host == "localhost"
}

func redirectWithParams(c *gin.Context, target *url.URL, params url.Values) {
	redirect := *target
	query := redirect.Query()
	for key, values := range params {
		query[key] = values
	}
	redirect.RawQuery = query.Encode()
	c.Redirect(http.StatusFound, redirect.String())
}

func (s *AuthorizationServer) rejectAuthorization(c *gin.Context, req authorizationRequest,
	errCode, errDescription string) {
	if req.redirectURI == nil {
		// Never redirect to an untrusted URI (RFC 6749, section 4.1.2.1).
		c.String(http.StatusBadRequest, "%s: %s", errCode, errDescription)
		return
	}
	redirectWithParams(c, req.redirectURI, url.Values{
		"error":             {errCode},
		"error_description": {errDescription},
		"state":             {req.state},
	})
}

var consentPage = template.Must(template.New("consent").Parse(`<!DOCTYPE html>
<html lang="en">
<head><meta charset="utf-8"><title>Qt OpenAPI demo authorization server</title>
<style>
body { font-family: sans-serif; max-width: 32em; margin: 3em auto; }
button { font-size: 1em; padding: 0.4em 1.2em; margin-right: 1em; }
</style></head>
<body>
<h1>Qt OpenAPI demo authorization server</h1>
<p>The application <strong>{{.ClientID}}</strong> asks for permission to:</p>
<ul>{{range .Scopes}}<li><code>{{.Name}}</code> - {{.Description}}</li>{{end}}</ul>
<form method="post" action="/oauth/authorize">
{{range $key, $value := .Params}}<input type="hidden" name="{{$key}}" value="{{$value}}">
{{end}}
<p><label>Your display name:
<input type="text" name="display_name" value="Qt Demo User"></label></p>
<button type="submit" name="decision" value="allow">Allow</button>
<button type="submit" name="decision" value="deny">Deny</button>
</form>
<p>The browser is redirected to <code>{{.RedirectURI}}</code> afterwards.</p>
</body></html>`))

// GET /oauth/authorize
func (s *AuthorizationServer) showConsentPage(c *gin.Context) {
	params := c.Request.URL.Query()
	req, errCode, errDescription := parseAuthorizationRequest(params)
	if errCode != "" {
		s.rejectAuthorization(c, req, errCode, errDescription)
		return
	}

	type scopeItem struct{ Name, Description string }
	var scopes []scopeItem
	for _, scope := range req.scope {
		scopes = append(scopes, scopeItem{scope, knownScopes[scope]})
	}
	hidden := make(map[string]string)
	for _, key := range []string{"client_id", "redirect_uri", "response_type",
	    "scope", "state", "code_challenge", "code_challenge_method"} {
		hidden[key] = params.Get(key)
	}

	c.Status(http.StatusOK)
	c.Header("Content-Type", "text/html; charset=utf-8")
	_ = consentPage.Execute(c.Writer, map[string]any{
		"ClientID":    ClientID,
		"Scopes":      scopes,
		"Params":      hidden,
		"RedirectURI": req.redirectURI.String(),
	})
}

// POST /oauth/authorize
func (s *AuthorizationServer) handleConsent(c *gin.Context) {
	if err := c.Request.ParseForm(); err != nil {
		c.String(http.StatusBadRequest, "invalid form")
		return
	}
	form := c.Request.PostForm
	req, errCode, errDescription := parseAuthorizationRequest(form)
	if errCode != "" {
		s.rejectAuthorization(c, req, errCode, errDescription)
		return
	}
	if form.Get("decision") != "allow" {
		s.rejectAuthorization(c, req, "access_denied", "the user denied the request")
		return
	}

	displayName := strings.TrimSpace(form.Get("display_name"))
	if displayName == "" {
		displayName = "Qt Demo User"
	}
	code := randomToken()
	s.mu.Lock()
	s.codes[code] = pendingCode{
		Session: Session{
			Scope: req.scope,
			User: UserProfile{
				Id:          "qt-demo-user",
				DisplayName: displayName,
				Email:       "demo@example.com",
				Country:     "FI",
			},
		},
		redirectURI:   form.Get("redirect_uri"),
		codeChallenge: req.codeChallenge,
		expiresAt:     time.Now().Add(authorizationCodeLifetime),
	}
	s.mu.Unlock()

	redirectWithParams(c, req.redirectURI, url.Values{"code": {code}, "state": {req.state}})
}

// POST /oauth/token
func (s *AuthorizationServer) issueToken(c *gin.Context) {
	// Error responses as described in RFC 6749, section 5.2.
	fail := func(errCode, errDescription string) {
		c.Header("Cache-Control", "no-store")
		c.JSON(http.StatusBadRequest, gin.H{"error": errCode, "error_description": errDescription})
	}

	if c.PostForm("client_id") != ClientID {
		fail("invalid_client", "unknown client_id")
		return
	}

	var session Session
	switch c.PostForm("grant_type") {
	case "authorization_code":
		s.mu.Lock()
		pending, ok := s.codes[c.PostForm("code")]
		delete(s.codes, c.PostForm("code")) // a code is single-use, whatever happens next
		s.mu.Unlock()
		if !ok || time.Now().After(pending.expiresAt) {
			fail("invalid_grant", "unknown or expired authorization code")
			return
		}
		if c.PostForm("redirect_uri") != pending.redirectURI {
			fail("invalid_grant", "redirect_uri does not match the authorization request")
			return
		}
		if !verifyPkce(c.PostForm("code_verifier"), pending.codeChallenge) {
			fail("invalid_grant", "code_verifier does not match code_challenge")
			return
		}
		session = pending.Session

	case "refresh_token":
		s.mu.Lock()
		refreshed, ok := s.refreshTokens[c.PostForm("refresh_token")]
		delete(s.refreshTokens, c.PostForm("refresh_token")) // refresh tokens rotate
		s.mu.Unlock()
		if !ok {
			fail("invalid_grant", "unknown refresh token, authorize again")
			return
		}
		session = refreshed

	default:
		fail("unsupported_grant_type", "use authorization_code or refresh_token")
		return
	}

	access, refresh := randomToken(), randomToken()
	s.mu.Lock()
	s.accessTokens[access] = accessToken{
		Session:   session,
		expiresAt: time.Now().Add(accessTokenLifetime),
	}
	s.refreshTokens[refresh] = session
	s.mu.Unlock()

	c.Header("Cache-Control", "no-store")
	c.JSON(http.StatusOK, gin.H{
		"access_token":  access,
		"token_type":    "Bearer",
		"expires_in":    int(accessTokenLifetime.Seconds()),
		"refresh_token": refresh,
		"scope":         strings.Join(session.Scope, " "),
	})
}

// verifyPkce checks code_verifier against code_challenge for the S256 method
// (RFC 7636, section 4.6).
func verifyPkce(verifier, challenge string) bool {
	if verifier == "" {
		return false
	}
	sum := sha256.Sum256([]byte(verifier))
	return base64.RawURLEncoding.EncodeToString(sum[:]) == challenge
}

func randomToken() string {
	buf := make([]byte, 32)
	if _, err := rand.Read(buf); err != nil {
		panic(err)
	}
	return base64.RawURLEncoding.EncodeToString(buf)
}
