//
//  ResearchConstraintSolver.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ResearchConstraintSolver__
#define __Stonefish_ResearchConstraintSolver__

#include <BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h>
#include <BulletDynamics/MLCPSolvers/btMLCPSolverInterface.h>
#include "StonefishCommon.h"

namespace sf
{
    //! A class implementing a custom constraint solver combining multibody dynamics with MLCP.
    class ResearchConstraintSolver : public btMultiBodyConstraintSolver
    {
    public:
        //! A constructor.
        /*!
         \param mlcp a pointer to an MLCP type solver
         */
        ResearchConstraintSolver(btMLCPSolverInterface* mlcp);
        
        //! A destructor.
        virtual ~ResearchConstraintSolver();
        
        //! A method running the MLCP solver.
        /*!
         \param infoGlobal a reference to the contact solver data
         */
        virtual bool solveMLCP(const btContactSolverInfo& infoGlobal);
        
        //! A method returning a number of fallbacks to the sequential impulse algorithm.
        int getNumFallbacks() const;
        
        //! A method resetting the number of fallbacks.
        void resetNumFallbacks();
        
        //! A method returning the solver type.
        virtual btConstraintSolverType getSolverType() const;
        
    protected:
        btMatrixXu m_A;
        btVectorXu m_b;
        btVectorXu m_x;
        btVectorXu m_lo;
        btVectorXu m_hi;
        
        ///when using 'split impulse' we solve two separate (M)LCPs
        btVectorXu m_bSplit;
        btVectorXu m_xSplit;
        btVectorXu m_bSplit1;
        btVectorXu m_xSplit2;
        
        btAlignedObjectArray<int> m_limitDependencies;
        btAlignedObjectArray<btSolverConstraint*> m_allConstraintPtrArray;
        btMLCPSolverInterface* m_solver;
        int m_fallback;
        Scalar m_cfm;
        bool m_gUseMatrixMultiply;
        bool m_interleaveContactAndFriction;
        
        virtual Scalar solveGroupCacheFriendlySetup(btCollisionObject** bodies, int numBodies, btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer);
        virtual Scalar solveGroupCacheFriendlyIterations(btCollisionObject** bodies ,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer);
        
        virtual void createMLCP(const btContactSolverInfo& infoGlobal);
        virtual void createMLCPFast(const btContactSolverInfo& infoGlobal);
    };
}

#endif
