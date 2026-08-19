#!/usr/bin/env python3
import html
import os
import sys
import time

def escape(value):
	return html.escape(value, quote=True)


def read_body():
	try:
		length = int(os.environ.get('CONTENT_LENGTH', '0') or '0')
	except ValueError:
		length = 0

	if length <= 0:
		return ''

	return sys.stdin.read(length)


method = os.environ.get('REQUEST_METHOD', 'UNKNOWN')
query_string = os.environ.get('QUERY_STRING', '')
content_length = os.environ.get('CONTENT_LENGTH', '0')
content_type = os.environ.get('CONTENT_TYPE', 'unknown')
body = read_body()
time.sleep(30)
response_body = (
	'Method: {}\n'
	'Query string: {}\n'
	'Content-Type: {}\n'
	'Content-Length: {}\n\n'
	'Raw body:\n{}'
).format(
	escape(method),
	escape(query_string or '(empty)'),
	escape(content_type),
	escape(content_length),
	escape(body or '(empty)')
)

print('Content-Type: text/html; charset=utf-8\n')
print('<!DOCTYPE html>')
print('<html lang="en">')
print('<head>')
print('    <meta charset="UTF-8">')
print('    <meta name="viewport" content="width=device-width, initial-scale=1.0">')
print('    <title>CGI Test</title>')
print('    <link rel="stylesheet" href="/css/style.css">')
print('</head>')
print('<body>')
print('    <div class="noise"></div>')
print('    <main class="container dashboard">')
print('        <header class="topbar">')
print('            <div class="brand">')
print('                <div class="brand-mark">CGI</div>')
print('                <div>')
print('                    <div class="tag tag-inline">✦ CGI tester • Python echo</div>')
print('                    <h1>request <span class="gradient">echo</span></h1>')
print('                </div>')
print('            </div>')
print('            <p class="hero-copy">This page mirrors the cgi_tester flow and shows exactly what the server passed into the Python script.</p>')
print('        </header>')
print('        <section class="response-shell">')
print('            <div class="response-topline">')
print('                <span class="response-title">cgi output</span>')
print('                <span class="response-badge">handled by python</span>')
print('            </div>')
print('            <div class="stats-row">')
print('                <article class="stat-card"><span class="stat-label">method</span><strong>{}</strong><small>request method</small></article>'.format(escape(method)))
print('                <article class="stat-card"><span class="stat-label">query string</span><strong>{}</strong><small>URL parameters</small></article>'.format(escape(query_string or '—')))
print('                <article class="stat-card"><span class="stat-label">content length</span><strong>{}</strong><small>posted bytes</small></article>'.format(escape(content_length)))
print('            </div>')
print('            <div class="response-shell" style="margin-top: 18px;">')
print('                <pre class="response-output">{}</pre>'.format(response_body))
print('            </div>')
print('            <div class="buttons" style="margin-top: 18px;">')
print('                <a class="btn primary" href="/">back to browser lab</a>')
print('                <a class="btn secondary" href="/test.py?source=direct">reload CGI</a>')
print('            </div>')
print('        </section>')
print('    </main>')
print('</body>')
print('</html>')
