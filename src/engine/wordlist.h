#pragma once
#include <unordered_set>
#include <string>

namespace KType {

// Level 1: VietType wlist_en (127 words) — common English words that collide with Telex
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

// Level 2: VietType wlist_en_2 + KType additions — extended list
// KType additions: missing singular/plural pairs for L1/L2 words,
// where Telex collision does NOT produce a common Vietnamese word.
// Skipped: beer→bể, car→cả, chair→chải, char→chả, bar→bả, tar→tả,
//   hair→hải, hoof→hồ, roof→rồ, moor→mổ, boor→bổ, goof→gồ, sex→sẽ,
//   moon→môn, queen→quên, teen→tên, tree→trê, barn→bản, sir→sỉ, vex→vẽ
inline const std::unordered_set<std::wstring>& WordListEn2() {
    static const std::unordered_set<std::wstring> words = {
        // --- VietType wlist_en_2 original ---
        L"ask", L"bask", L"bays", L"bias", L"bins", L"boar", L"boas", L"boast",
        L"boats", L"books", L"booms", L"bore", L"born", L"bosom", L"bums", L"bury",
        L"busy", L"buys", L"cask", L"chaps", L"charm", L"chasm", L"cheeks", L"cheeps",
        L"cheer", L"choir", L"chore", L"chosen", L"coast", L"coats", L"coax", L"cons",
        L"cooks", L"core", L"cox", L"darn", L"dawns", L"deem", L"deems", L"deeps",
        L"dens", L"dense", L"desk", L"dins", L"disc", L"disk", L"doer", L"does",
        L"donor", L"doom", L"dooms", L"door", L"dose", L"dosed", L"down", L"downs",
        L"dusk", L"ekes", L"gangs", L"gawks", L"gee", L"gees", L"gems", L"gene",
        L"genes", L"genre", L"germ", L"gets", L"ghost", L"gins", L"gist", L"goats",
        L"goes", L"gongs", L"goons", L"gore", L"gown", L"gowns", L"gums", L"guns",
        L"guys", L"hawks", L"hems", L"hens", L"her", L"hims", L"hoax", L"hoes",
        L"hooks", L"hoops", L"hose", L"hums", L"husk", L"keen", L"keens", L"kings",
        L"koran", L"lawns", L"leeks", L"liar", L"lix", L"loans", L"looks", L"loon",
        L"lore", L"mamas", L"maps", L"mask", L"meets", L"mere", L"metes", L"moans",
        L"moats", L"moons", L"more", L"morn", L"moron", L"musk", L"naps", L"nieces",
        L"noes", L"nooks", L"norm", L"nose", L"nuns", L"oaf", L"oaks", L"oar",
        L"oks", L"or", L"ox", L"oxen", L"pair", L"pangs", L"pans", L"papas",
        L"par", L"pas", L"past", L"pasta", L"pats", L"pawn", L"pawns", L"pays",
        L"peek", L"peeks", L"peeps", L"peer", L"penes", L"pens", L"peps", L"per", L"pest",
        L"pets", L"photos", L"pieces", L"pins", L"pis", L"pits", L"poems", L"poets",
        L"poops", L"poor", L"pops", L"pore", L"pose", L"post", L"pots", L"puns",
        L"pups", L"puree", L"pus", L"puts", L"quays", L"queens", L"queer", L"quips",
        L"reeks", L"reuse", L"rims", L"rings", L"rips", L"risk", L"roams", L"roar",
        L"roast", L"roes", L"rooks", L"rooms", L"rose", L"says", L"seeks", L"seem",
        L"seems", L"sees", L"sics", L"sings", L"sins", L"sips", L"soaks", L"soaps",
        L"soar", L"soon", L"sops", L"sore", L"sos", L"sox", L"sums", L"task",
        L"teems", L"teens", L"tens", L"tense", L"themes", L"things", L"thongs", L"thorn",
        L"those", L"tings", L"tongs", L"tons", L"tore", L"torn", L"town", L"trays",
        L"trees", L"treks", L"trims", L"trips", L"troops", L"tureen", L"tusk", L"veer",
        L"vips", L"xix",
        // --- KType: missing singulars for ee→ê collisions ---
        L"cheek", L"cheep", L"deep", L"deer", L"leek", L"leer", L"meet", L"peep",
        L"reek", L"rook", L"see", L"seek",
        // --- KType: missing singulars for oo→ô collisions ---
        // Skipped: room→rôm (rôm sảy = prickly heat)
        L"book", L"boom", L"cook", L"hook", L"hoop", L"look", L"nook", L"poop",
        L"troop",
        // --- KType: missing singulars for r→hỏi tone collisions ---
        // Skipped: arm→ảm (ảm đạm), burn→bủn (bủn rủn), corn→cỏn (cỏn con),
        //   honor→hổn (hổn hển), lair→lải (lải nhải), turn→tủn (tủn mủn)
        L"churn", L"harm", L"horn", L"motor", L"ore", L"rotor", L"term", L"urn",
        // --- KType: other missing singulars/forms ---
        // Skipped: reef→rề (rề rà)
        L"leper", L"meter", L"metre", L"pester", L"peter", L"tester",
    };
    return words;
}

} // namespace KType
