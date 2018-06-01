const globalEval = eval;
const window = globalEval(this);
window['foo'] = () => {
  //V8Worker2.print("Hello world from main.ts");
  return "foo";
}
