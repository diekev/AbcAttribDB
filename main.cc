
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/AbcGeom/All.h>

#include <filesystem>
#include <fstream>

using namespace Alembic;
using namespace Alembic::AbcCoreAbstract;
using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;


struct Attribut {
    std::string nom;
    std::string type; // float, float2, etc.
    std::string portee;
};

struct DonneesObjet {
    std::string type; // IPolyMesh, etc.

    std::vector<Attribut> attributs;
};

struct DonneesFichier {
    std::filesystem::path chemin;
    bool possede_instances = false;

    std::vector<DonneesObjet> objets;
};

static bool est_fichier_alembic(const std::filesystem::path &chemin)
{
    if (!std::filesystem::is_regular_file(chemin)) {
        return false;
    }

    const auto &extension = chemin.extension();

    if (extension != ".abc" && extension != ".ABC") {
        return false;
    }

    return true;
}

static std::string string_depuis_type(DataType dt)
{
    std::stringstream ss;
    ss << dt;
    return ss.str();
}

static std::string string_depuis_portee(GeometryScope scope)
{
    if (scope == kConstantScope) {
        return "kConstantScope";
    }
    if (scope == kUniformScope) {
        return "kUniformScope";
    }
    if (scope == kVaryingScope) {
        return "kVaryingScope";
    }
    if (scope == kVertexScope) {
        return "kVertexScope";
    }
    if (scope == kFacevaryingScope) {
        return "kFacevaryingScope";
    }

    return "kUnknownScope";
}

template <typename TypeParam>
static Attribut attribut_depuis_param(TypeParam param)
{
    auto attribut = Attribut();
    attribut.nom = param.getName();
    attribut.type = string_depuis_type(param.getDataType());
    attribut.portee = string_depuis_portee(param.getScope());
    return attribut;
}

static Attribut attribut_depuis_entete(const ICompoundProperty &parent, const PropertyHeader &propHeader)
{
    if (IFloatGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IFloatGeomParam(parent, propHeader.getName()));
    }

    if (IDoubleGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IDoubleGeomParam(parent, propHeader.getName()));
    }

    if (IV3dGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IV3dGeomParam(parent, propHeader.getName()));
    }

    if (IInt32GeomParam::matches(propHeader)) {
        return attribut_depuis_param(IInt32GeomParam(parent, propHeader.getName()));
    }

    if (IStringGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IStringGeomParam(parent, propHeader.getName()));
    }

    if (IV2fGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IV2fGeomParam(parent, propHeader.getName()));
    }

    if (IV3fGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IV3fGeomParam(parent, propHeader.getName()));
    }

    if (IP3fGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IP3fGeomParam(parent, propHeader.getName()));
    }

    if (IP3dGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IP3dGeomParam(parent, propHeader.getName()));
    }

    if (IN3fGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IN3fGeomParam(parent, propHeader.getName()));
    }

    if (IC3fGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IC3fGeomParam(parent, propHeader.getName()));
    }

    if (IM44fGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IM44fGeomParam(parent, propHeader.getName()));
    }

    if (IBoolGeomParam::matches(propHeader)) {
        return attribut_depuis_param(IBoolGeomParam(parent, propHeader.getName()));
    }

    return {};
}

static void parse_attributs(DonneesObjet &objet, ICompoundProperty props)
{
    if (!props) {
        return;
    }

    for (size_t i = 0; i < props.getNumProperties(); ++i) {
        auto &prop = props.getPropertyHeader(i);

        if (prop.isCompound()) {
            parse_attributs(objet, ICompoundProperty(props, prop.getName()));
        }
        else {
            //auto type = prop.getPropertyType(); // compound, scalar, array 0 1 2
            auto attribut = attribut_depuis_entete(props, prop);

            if (attribut.nom != "" && attribut.type != "" && attribut.portee != "") {
                objet.attributs.push_back(attribut);
            }
        }
    }
}

static void imprime_objets(DonneesFichier &donnees_fichier)
{
    std::cerr << "Le fichier possède " << donnees_fichier.objets.size() << " objets\n";
    for (auto &objet : donnees_fichier.objets) {
        std::cerr << "Objet " << objet.type << " a " << objet.attributs.size() << " attributs !\n";

        for (auto a : objet.attributs) {
            std::cerr << "-- " << a.nom << ", " << a.type << ", " << a.portee << "\n";
        }
    }
}

static void ajoute_objet(DonneesFichier &donnees_fichier, IObject top, std::string type)
{
    auto objet = DonneesObjet{};
    objet.type = type;
    parse_attributs(objet, top.getProperties());
    donnees_fichier.objets.push_back(objet);
}

static void traverse_hierarchie(DonneesFichier &donnees_fichier, IObject top)
{
    if (IPolyMesh::matches(top.getHeader())) {
        ajoute_objet(donnees_fichier, top, "IPolyMesh");
    }
    else if (ISubD::matches(top.getHeader())) {
        ajoute_objet(donnees_fichier, top, "ISubD");
    }
    else if (ICurves::matches(top.getHeader())) {
        ajoute_objet(donnees_fichier, top, "ICurves");
    }
    else if (IXform::matches(top.getHeader())) {
        ajoute_objet(donnees_fichier, top, "IXform");
    }
    else if (IFaceSet::matches(top.getHeader())) {
        ajoute_objet(donnees_fichier, top, "IFaceSet");
    }
    else if (INuPatch::matches(top.getHeader())) {
        ajoute_objet(donnees_fichier, top, "INuPatch");
    }
    else if (IPoints::matches(top.getHeader())) {
        ajoute_objet(donnees_fichier, top, "INuPatch");
    }
    else {
        if (top.isInstanceRoot()) {
            donnees_fichier.possede_instances = true;
        }
    }

    for (auto i = 0; i < top.getNumChildren(); ++i) {
        auto child = top.getChild(i);
        traverse_hierarchie(donnees_fichier, child);
    }
}

static void analyse_fichier_alembic(const std::filesystem::path &chemin, std::vector<DonneesFichier> &fichiers)
{
    Alembic::AbcCoreFactory::IFactory factory;
    factory.setPolicy(Alembic::Abc::ErrorHandler::kQuietNoopPolicy);
    auto archive = factory.getArchive(chemin.c_str());

    if (!archive.valid()) {
        std::cerr << "Cannot open " << chemin << " for reading !\n";
        return;
    }

    auto donnees_fichier = DonneesFichier{};
    donnees_fichier.chemin = chemin;

    std::cout << "Analyse de " << chemin << '\n';
    traverse_hierarchie(donnees_fichier, archive.getTop());

    fichiers.push_back(donnees_fichier);
}

#define GUILLEMET(x) '"' << x << '"'

static void imprime_fichier_pour_fichiers(const std::vector<DonneesFichier> &fichiers, std::ostream &os)
{
    // imprime les fichiers
    // id_fichier, chemin
    os << "id,chemin\n";
    int id_fichier = 0;
    for (auto &fichier: fichiers) {
        //imprime_objets(fichier);
        os << id_fichier++ << "," << fichier.chemin << "\n";
    }
}

static void imprime_fichier_pour_objets(const std::vector<DonneesFichier> &fichiers, std::ostream &os)
{
    // imprime les objets
    // id_fichier, id_objet, type
    os << "id,id_fichier,type\n";
    int id_fichier = 0;
    int id_objet = 0;
    for (auto &fichier: fichiers) {
        for (auto &objet: fichier.objets) {
            os << id_objet++ << "," << id_fichier << "," << GUILLEMET(objet.type) << "\n";
        }

        id_fichier++;
    }
}

static void imprime_fichier_pour_attributs(const std::vector<DonneesFichier> &fichiers, std::ostream &os)
{
    // imprime les attributs
    // id_objet, id_attribut, nom, type, portee
    os << "id,id_objet,nom,type,portee\n";
    int id_fichier = 0;
    int id_objet = 0;
    int id_attribut = 0;
    for (auto &fichier: fichiers) {
        for (auto &objet: fichier.objets) {
            for (auto &attribut: objet.attributs) {
                os << id_attribut++ << "," << id_objet << "," << GUILLEMET(attribut.nom) << "," << GUILLEMET(attribut.type) << "," << GUILLEMET(attribut.portee) << "\n";
            }

            id_objet++;
        }

        id_fichier++;
    }
}

int main(int argc, const char **argv)
{
    if (argc != 2) {
        std::cerr << "Utilisation : " << argv[0] << " DOSSIER\n";
        return 1;
    }

    auto chemin = std::filesystem::path(argv[1]);

    std::vector<DonneesFichier> fichiers;

    for (auto entry : std::filesystem::recursive_directory_iterator(chemin)) {
        if (!est_fichier_alembic(entry.path())) {
            continue;
        }

        analyse_fichier_alembic(entry.path(), fichiers);
    }

    std::cout << "////////////////////////////////\n";
    std::cout << "Impression du fichier de fichiers...\n";
    std::ofstream fichier_fichiers("/tmp/abc_fichiers.csv");
    imprime_fichier_pour_fichiers(fichiers, fichier_fichiers);

    std::cout << "////////////////////////////////\n";
    std::cout << "Impression du fichier d'objets...\n";
    std::ofstream fichier_objets("/tmp/abc_objets.csv");
    imprime_fichier_pour_objets(fichiers, fichier_objets);

    std::cout << "////////////////////////////////\n";
    std::cout << "Impression du fichier d'attributs...\n";
    std::ofstream fichier_attributs("/tmp/abc_attributs.csv");
    imprime_fichier_pour_attributs(fichiers, fichier_attributs);

    return 0;
}
