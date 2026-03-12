#pragma once
#include <unordered_set>
#include <string>

namespace KType {

// English words that collide with Vietnamese Telex input.
// Based on VietType wlist_en (127 words).
// Words whose Telex result produces common Vietnamese morphemes are excluded.
inline const std::unordered_set<std::wstring>& WordListEn() {
    static const std::unordered_set<std::wstring> words = {
        L"airs", L"arms", L"auras", L"axis", L"barns", L"bars", L"beefs", L"beers",
        L"boars", L"boors", L"bores", L"boxer", L"boxers", L"boxes", L"burns", L"bursar",
        L"burst", L"cars", L"chairs", L"charms", L"chars", L"cheers", L"chefs", L"chiefest",
        L"choirs", L"chores", L"churns", L"cores", L"corns", L"corset", L"curst", L"darns",
        L"deers", L"defends", L"defer", L"defers", L"denser", L"deters", L"doers", L"donors",
        L"doors", L"genres", L"germs", L"goofs", L"gores", L"hairs", L"hangars", L"harms",
        L"heros", L"hers", L"honors", L"hoofs", L"horns", L"horse", L"ifs", L"irs",
        L"korans", L"lairs", L"leers", L"lepers", L"liars", L"loafs", L"loser", L"losers",
        L"major", L"majors", L"mars", L"meres", L"merest", L"meters", L"metres", L"moors",
        L"morns", L"morons", L"motors", L"norms", L"oafs", L"oars", L"ores", L"pairs",
        L"pars", L"peers", L"perjure", L"perjures", L"peruse", L"pesters", L"peters", L"pores",
        L"purees", L"queers", L"reefs", L"refer", L"refers", L"refuse", L"roars", L"roofs",
        L"rosary", L"rotors", L"saris", L"sexes", L"sirs", L"soars", L"sofas", L"sores",
        L"sorest", L"surf", L"surfs", L"tars", L"taxis", L"tenser", L"terms", L"terse",
        L"terser", L"testers", L"thirst", L"thorns", L"torsi", L"torso", L"tureens", L"turf",
        L"turfs", L"turns", L"urns", L"veers", L"verse", L"vexes", L"virus",
    };
    return words;
}

} // namespace KType
