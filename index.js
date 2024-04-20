const express = require("express");
const path = require("path");
const bot = require("./build/Release/bot");

const app = express();
const port = 3000;

app.use(express.static(path.join(__dirname, "/src/")));
app.use(express.json());

app.get("/", function (req, res) {
  res.sendFile(path.join(__dirname, '/src/home.html'));
});

app.post("/bestMove", function(req, res) {
  move = bot.getBestMove(req.body.fen);
  //console.log(move);
  res.send(JSON.stringify({
    move
  }));
  res.end();
});

app.post("/initialposition", function(req, res) {
  bot.setFen(req.body.fen);
  res.sendStatus(200).end();
});

app.post("/move", function(req, res) {
  bot.doMove(req.body.san);
  res.end();
});

app.listen(port, function () {
  console.log(`Virtuoso listening on port ${port}!`);
});
