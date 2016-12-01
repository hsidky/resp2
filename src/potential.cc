#include "Python.h"
#include <liboptions/liboptions.h>
#include <libmints/mints.h>
#include <libmints/oeprop.h>
#include <libpsio/psio.hpp>
using namespace psi;

std::vector<double> calculate_esp_at_points(SharedWavefunction& ref_wfn, std::vector<Vector3> points) {
    boost::shared_ptr<BasisSet> basisset = ref_wfn->basisset();
    boost::shared_ptr<Molecule> mol = basisset->molecule();
    boost::shared_ptr<IntegralFactory> integral_ = boost::shared_ptr<IntegralFactory>(new IntegralFactory(basisset, basisset, basisset, basisset));
    boost::shared_ptr<ElectrostaticInt> epot(dynamic_cast<ElectrostaticInt*>(integral_->electrostatic()));
    boost::shared_ptr<OEProp> oeprop = boost::shared_ptr<OEProp>(new OEProp(ref_wfn));

    int n_atoms = mol->natom();
    int nbf = basisset->nbf();
    std::vector<double> esp_values;

    SharedMatrix Dtot = oeprop->Da_ao();
    if (ref_wfn->same_a_b_dens()) {
        Dtot->scale(2.0);
    } else {
        Dtot->add(oeprop->Db_ao());
    }

    // compute the electrostatic potential at each of the points
    // this code probably should be in a different function?
    outfile->Printf("\n Electrostatic potentials at van der Waals shells:\n");
    outfile->Printf(" ---------------------------------------------------\n");
    outfile->Printf("   x   y   z  Electrostatic Potential (a.u.)\n");
    outfile->Printf(" ---------------------------------------------------\n");
    for (size_t i = 0; i < points.size(); i++) {
        std::stringstream s;
        SharedMatrix ints(new Matrix(s.str(), nbf, nbf));
        epot->compute(ints, points[i]);
        double elec = Dtot->vector_dot(ints);
        double nuc = 0.0;

        for (int atom1 = 0; atom1 < n_atoms; atom1++)
            nuc += (mol->Z(atom1) / points[i].distance(mol->xyz(atom1)));

        esp_values.push_back(nuc+elec);
        outfile->Printf("     %8.5f %8.5f %8.5f    %16.12f\n", points[i][0], points[i][1], points[i][2], nuc+elec);
    }
    outfile->Printf(" ---------------------------------------------------\n");
    outfile->Flush();

    return esp_values;
}
