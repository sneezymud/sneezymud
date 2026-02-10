"""Post a Discord notification when a new game image is published.

Environment variables (set in workflow step):
    DISCORD_NEW_BUILD_WEBHOOK_URL  - Discord webhook endpoint
    DISCORD_ADMIN_ROLE_ID          - Discord role ID to mention (optional)
    GH_TOKEN                       - GitHub token (for gh CLI auth)

Environment variables (provided automatically by GitHub Actions runner):
    GITHUB_REPOSITORY              - owner/repo (e.g. sneezymud/sneezymud)
    GITHUB_SHA                     - commit SHA that triggered the build
"""

import json
import os
import subprocess
import urllib.request

repo = os.environ["GITHUB_REPOSITORY"]
sha = os.environ["GITHUB_SHA"]
webhook_url = os.environ["DISCORD_NEW_BUILD_WEBHOOK_URL"]

admin_role_id = os.environ.get("DISCORD_ADMIN_ROLE_ID", "")

mention = f"<@&{admin_role_id}> " if admin_role_id else ""
msg = f"{mention}New game image available! Reboot to apply the update."
try:
    result = subprocess.run(
        ["gh", "api", f"repos/{repo}/commits/{sha}/pulls", "--jq", ".[0] // empty"],
        capture_output=True, text=True, check=True,
    )
    if result.stdout.strip():
        pr = json.loads(result.stdout)
        msg += f"\nSummary: {pr['title']} ({pr['html_url']})"
except (subprocess.CalledProcessError, json.JSONDecodeError, KeyError):
    pass

req = urllib.request.Request(
    webhook_url,
    data=json.dumps({
        "content": msg,
        "allowed_mentions": {"roles": [admin_role_id]} if admin_role_id else {},
    }).encode(),
    headers={"Content-Type": "application/json", "User-Agent": "SneezyMUD-CI"},
)
urllib.request.urlopen(req, timeout=10)
