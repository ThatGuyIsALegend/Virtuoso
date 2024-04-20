cmd_Release/bot.node := ln -f "Release/obj.target/bot.node" "Release/bot.node" 2>/dev/null || (rm -rf "Release/bot.node" && cp -af "Release/obj.target/bot.node" "Release/bot.node")
