# Smart Card Browser Extension

## Native Messages

Messages to native:
```
{i: 'string', c: integer, r: integer, a: 'string', p: integer}
```
i: unique message identifier
c: command 1-list readers, 2-connect, 3-disconnect, 4-transcieve
r: index of reader in reader list
a: hex cAPDU to send to the card, sent only for c: 3
p: parameter, share mode for connect, sent only for c: 1

Messages from native:
```
{i: 'string', e: integer, r: integer, d: [array]|'string'}
```
i: unique message identifier, to link the response. Empty string on reader events
e: reader event 1-card insert, 2-card remove. Sent only for reader events
r: reader index for reader events
d: data 1-string array with list of readers, 2-card atr, 4-hex rAPDU