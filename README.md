# save-last-active-firefox-profile

This repo for tracking the last active profile and storing the profile name in a file.
We can use it to open external url to the last active profile, just like google chrome.

```shell
# update /usr/share/applications/firefox.desktop or /var/lib/snapd/desktop/applications/firefox_firefox.desktop
# depends on which file is set for default browser
Exec=/yourpath/firefox -P "$(result=$(head -n 1 /yourpath/output.txt); [ -z "$result" ] && echo "default" || echo "$result")" -new-tab %u
```
TODO
- [ ] update the regex regarding to firefox path https://github.com/linhx/save-last-active-firefox-profile/blob/877f60e14dcd0109a627448b236760f32234bc77/main.c#L50C83-L50C83
