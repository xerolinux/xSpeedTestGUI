#include <QFile>
#include <QJSEngine>
#include <QtTest>

class VisualizersTest : public QObject
{
    Q_OBJECT

    static QString harness()
    {
        return QStringLiteral(R"js(
function runAll() {
    var bad = [];
    var names = Object.keys(V);
    var theme = {d: ["#4fd1ff", "#2f6bff"], u: ["#7cf7d4", "#19b5a5"], p: ["#8f98ad", "#c7cee0"],
                 track: "rgba(255,255,255,.12)", fg: "#eef1f8", dim: "rgba(238,241,248,.6)"};
    var sizes = [[350, 200], [180, 150], [400, 64], [230, 150]];
    var hist = []; for (var i = 0; i < 70; i++) hist.push(Math.min(1, i / 70));
    var scenarios = [
        {ch: "idle", f: 0, p: 0, active: false, hist: []},
        {ch: "ping", f: .07, p: .5, active: true, hist: []},
        {ch: "down", f: .5, p: .4, active: true, hist: hist},
        {ch: "up", f: 1, p: 1, active: true, hist: hist},
        {ch: "down", f: 0, p: 1, active: false, hist: hist}
    ];
    function check(name, args) {
        for (var i = 0; i < args.length; i++)
            if (typeof args[i] === "number" && !isFinite(args[i])) bad.push(name + ": non-finite argument to a draw call");
    }
    function makeCtx(name) {
        var gradient = {addColorStop: function (o, c) { if (typeof c !== "string" || !isFinite(o)) bad.push(name + ": bad color stop"); }};
        return new Proxy({}, {
            get: function (t, prop) {
                if (prop in t) return t[prop];
                if (/^create/.test(prop)) return function () { check(name, arguments); return gradient; };
                return function () { check(name, arguments); };
            },
            set: function (t, prop, value) {
                if (typeof value === "number" && !isFinite(value)) bad.push(name + ": non-finite " + prop);
                t[prop] = value; return true;
            }
        });
    }
    names.forEach(function (name) {
        sizes.forEach(function (size) {
            var draw = V[name].make();
            scenarios.forEach(function (sc) {
                for (var frame = 0; frame < 20; frame++) {
                    var s = {ch: sc.ch, f: sc.f, p: sc.p, active: sc.active, hist: sc.hist, t: frame * .016, dt: .016, pd: sc.p, pu: sc.p / 2};
                    try { draw(makeCtx(name), size[0], size[1], s, theme); }
                    catch (e) { bad.push(name + ": " + e); return; }
                }
            });
        });
    });
    return {count: names.length, bad: bad.slice(0, 5)};
}
)js");
    }

private slots:
    void everyVisualizerDrawsFiniteValuesInEveryState()
    {
        QFile file(QStringLiteral(VISUALIZERS_JS));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QString source = QString::fromUtf8(file.readAll());
        source.remove(QStringLiteral(".pragma library"));

        QJSEngine engine;
        const QJSValue loaded = engine.evaluate(source + harness());
        QVERIFY2(!loaded.isError(), qPrintable(loaded.toString()));
        const QJSValue report = engine.evaluate(QStringLiteral("runAll()"));
        QVERIFY2(!report.isError(), qPrintable(report.toString()));
        QCOMPARE(report.property("count").toInt(), 21);
        QCOMPARE(report.property("bad").property("length").toInt(), 0);
        if (report.property("bad").property("length").toInt() > 0)
            qWarning() << report.property("bad").toVariant();
    }
};

QTEST_GUILESS_MAIN(VisualizersTest)
#include "tst_visualizers.moc"
