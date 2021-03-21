/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier . hosxe (at) gmail . com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Osc.h"

float silence[1]  __attribute__ ((section(".ccmnoload")));
float noise[32] __attribute__ ((section(".ccmnoload")));

#include "../waveforms/waves.c"

float *frequencyToUse;

float* Osc::oscValues[4] __attribute__ ((section(".ccmnoload")));
float oscValues1[32] __attribute__ ((section(".ccmnoload")));
float oscValues2[32] __attribute__ ((section(".ccmnoload")));
float oscValues3[32] __attribute__ ((section(".ccmnoload")));
float oscValues4[32] __attribute__ ((section(".ccmnoload")));
int Osc::oscValuesCpt = 1;

// User waveforms
float userWaveform[6][1024];

struct WaveTable waveTables[NUMBER_OF_WAVETABLES] __attribute__ ((section(".ccm"))) = {
        //		OSC_SHAPE_SIN = 0,
        {
                sinTable,
                0x7ff,
                1.0f,
                0.0f,
                0.0f
        },
        //		OSC_SHAPE_SAW,
        {
                sawTable,
                0x7ff,
                1.0f,
                0.0f,
                0.0f
        },
        //		OSC_SHAPE_SQUARE,
        {
                squareTable,
                0x7ff,
                1.0f,
                0.0f,
                0.0f
        },
        //		OSC_SHAPE_SIN_SQUARE,
        {
                sinSquareTable,
                0x3ff,
                1.0f,
                0.0f,
                0.0f
        },
        //		OSC_SHAPE_SIN_ZERO,
        {
                sinOrZeroTable,
                0x3ff,
                1.0f,
                0.0f,
                0.0f
        },
        //		OSC_SHAPE_SIN_POS,
        {
                sinPosTable,
                0x3ff,
                1.0f,
                0.0f,
                0.0f
        },
        //		OSC_SHAPE_RAND,
        {
                noise,
                0x1f,
                0.0f,
                1.0f,
                0.0f
        },
        //	OSC_SHAPE_OFF,
        {
                silence,
                0x00, // Must be 0 to reset index when switching from an other shape.
                0.0f,
                0.0f,
                0.0f
        },
        //  USER waveform 1
        {
                userWaveform[0],
                0x3ff,
                1.0f,
                0.0f,
                0.0f
        },
        //  USER waveform 2
        {
                userWaveform[1],
                0x3ff,
                1.0f,
                0.0f,
                0.0f
        },
        //  USER waveform 3
        {
                userWaveform[2],
                0x3ff,
                1.0f,
                0.0f,
                0.0f
        },
        //  USER waveform 4
        {
                userWaveform[3],
                0x3ff,
                1.0f,
                0.0f,
                0.0f
        },
        //  USER waveform 5
        {
                userWaveform[4],
                0x3ff,
                1.0f,
                0.0f,
                0.0f
        },
        //  USER waveform 6
        {
                userWaveform[5],
                0x3ff,
                1.0f,
                0.0f,
                0.0f
        }

};


void Osc::init(SynthState* sState, struct OscillatorParams *oscParams, DestinationEnum df) {
    
    this->synthState = sState;
    silence[0] = 0;

    this->destFreq = df;
    this->oscillator = oscParams;

    if (waveTables[0].precomputedValue <= 0) {
        for (int k=0; k<NUMBER_OF_WAVETABLES; k++) {
            waveTables[k].precomputedValue = (waveTables[k].max + 1) * waveTables[k].useFreq * PREENFM_FREQUENCY_INVERSED;
        }
        // Set frequencyToUse  to frequency (no Scala scale defined)
        frequencyToUse = frequency;
    }
    if (oscValuesCpt == 1) {
        oscValues[0] = oscValues1;
        oscValues[1] = oscValues2;
        oscValues[2] = oscValues3;
        oscValues[3] = oscValues4;
        oscValuesCpt = 0;
    }
}


#ifdef CVIN
void Osc::updateFreqFromCv(struct OscState* oscState, float freq, float noteFrequencyUnison) {
    setMainFrequency(oscState->mainFrequency, freq * noteFrequencyUnison);
}


void Osc::newNoteFromCv(struct OscState* oscState, float freq, float noteFrequencyUnison, float phase) {
    oscState->index = waveTables[(int) oscillator->shape].max * phase;
    setMainFrequency(oscState->mainFrequency, freq * noteFrequencyUnison);
    oscState->frequency = oscState->mainFrequency;
}
#endif


void Osc::newNote(struct OscState* oscState, int note, float noteFrequencyUnison, float phase) {
    oscState->index = waveTables[(int) oscillator->shape].max * phase;
    setMainFrequency(oscState->mainFrequency, frequencyToUse[note] * noteFrequencyUnison);
    oscState->frequency = oscState->mainFrequency;
}

void Osc::glideToNote(struct OscState* oscState, int note, float noteFrequencyUnison) {
    setMainFrequency(oscState->nextFrequency, frequencyToUse[note] * noteFrequencyUnison);
    oscState->fromFrequency = oscState->mainFrequency;
}

void Osc::glideStep(struct OscState* oscState, float phase) {
    oscState->mainFrequency = oscState->fromFrequency * (1 - phase) + oscState->nextFrequency * phase;
}

float exp2_harm[] = {
        /*-51.2*/ 0.051952368508924235,
        /*-51.1*/ 0.05225332551803182, /*-51*/ 0.052556025953357156, /*-50.9*/ 0.05286047991446555, /*-50.8*/ 0.05316669755942848, /*-50.7*/ 0.05347468910516262,
        /*-50.6*/ 0.05378446482777063, /*-50.5*/ 0.05409603506288395, /*-50.4*/ 0.05440941020600775, /*-50.3*/ 0.05472460071286783, /*-50.2*/ 0.05504161709975927,
        /*-50.1*/ 0.055360469943897546, /*-50*/ 0.0556811698837712, /*-49.9*/ 0.05600372761949703, /*-49.8*/ 0.05632815391317691, /*-49.7*/ 0.056654459589256904,
        /*-49.6*/ 0.056982655534888536, /*-49.5*/ 0.05731275270029195, /*-49.4*/ 0.05764476209912122, /*-49.3*/ 0.05797869480883184, /*-49.2*/ 0.05831456197105044,
        /*-49.1*/ 0.05865237479194644, /*-49*/ 0.05899214454260585, /*-48.9*/ 0.05933388255940744, /*-48.8*/ 0.059677600244401045, /*-48.7*/ 0.06002330906568776,
        /*-48.6*/ 0.06037102055780286, /*-48.5*/ 0.06072074632210035, /*-48.4*/ 0.061072498027140376, /*-48.3*/ 0.06142628740907821, /*-48.2*/ 0.06178212627205601,
        /*-48.1*/ 0.06214002648859669, /*-48*/ 0.0625, /*-47.9*/ 0.06286205881674084, /*-47.8*/ 0.06322621501887017, /*-47.7*/ 0.0635924807564179,
        /*-47.6*/ 0.06396086824979842, /*-47.5*/ 0.06433138979021824, /*-47.4*/ 0.06470405774008611, /*-47.3*/ 0.0650788845334254, /*-47.2*/ 0.06545588267628916,
        /*-47.1*/ 0.06583506474717724, /*-47*/ 0.06621644339745596, /*-46.9*/ 0.06660003135178028, /*-46.8*/ 0.06698584140851832, /*-46.7*/ 0.06737388644017843,
        /*-46.6*/ 0.06776417939383865, /*-46.5*/ 0.0681567332915786, /*-46.4*/ 0.06855156123091412, /*-46.3*/ 0.06894867638523418, /*-46.2*/ 0.0693480920042403,
        /*-46.1*/ 0.06974982141438894, /*-46*/ 0.0701538780193358, /*-45.9*/ 0.07056027530038321, /*-45.8*/ 0.07096902681692985, /*-45.7*/ 0.0713801462069232,
        /*-45.6*/ 0.07179364718731468, /*-45.5*/ 0.07220954355451707, /*-45.4*/ 0.07262784918486498, /*-45.3*/ 0.07304857803507785, /*-45.2*/ 0.07347174414272553,
        /*-45.1*/ 0.07389736162669676, /*-45*/ 0.07432544468767006, /*-44.9*/ 0.07475600760858774, /*-44.8*/ 0.07518906475513232, /*-44.7*/ 0.07562463057620579,
        /*-44.6*/ 0.07606271960441192, /*-44.5*/ 0.07650334645654094, /*-44.4*/ 0.07694652583405728, /*-44.3*/ 0.07739227252359003, /*-44.2*/ 0.07784060139742642,
        /*-44.1*/ 0.07829152741400798, /*-44*/ 0.07874506561842957, /*-43.9*/ 0.07920123114294139, /*-43.8*/ 0.07966003920745389, /*-43.7*/ 0.08012150512004554,
        /*-43.6*/ 0.0805856442774737, /*-43.5*/ 0.0810524721656881, /*-43.4*/ 0.08152200436034776, /*-43.3*/ 0.08199425652734058, /*-43.2*/ 0.08246924442330589,
        /*-43.1*/ 0.0829469838961605, /*-43*/ 0.08342749088562713, /*-42.9*/ 0.08391078142376648, /*-42.8*/ 0.08439687163551189, /*-42.7*/ 0.08488577773920757,
        /*-42.6*/ 0.0853775160471497, /*-42.5*/ 0.08587210296613058, /*-42.4*/ 0.08636955499798601, /*-42.3*/ 0.08686988874014608, /*-42.2*/ 0.08737312088618869,
        /*-42.1*/ 0.08787926822639684, /*-42*/ 0.08838834764831845, /*-41.9*/ 0.0889003761373301, /*-41.8*/ 0.08941537077720367, /*-41.7*/ 0.08993334875067624,
        /*-41.6*/ 0.09045432734002362, /*-41.5*/ 0.0909783239276367, /*-41.4*/ 0.0915053559966016, /*-41.3*/ 0.09203544113128287, /*-41.2*/ 0.09256859701791025,
        /*-41.1*/ 0.09310484144516888, /*-41*/ 0.09364419230479261, /*-40.9*/ 0.094186667592161, /*-40.8*/ 0.09473228540689989, /*-40.7*/ 0.0952810639534851,
        /*-40.6*/ 0.09583302154185004, /*-40.5*/ 0.0963881765879963, /*-40.4*/ 0.09694654761460841, /*-40.3*/ 0.09750815325167171, /*-40.2*/ 0.09807301223709383,
        /*-40.1*/ 0.09864114341733018, /*-40*/ 0.09921256574801246, /*-39.9*/ 0.09978729829458126, /*-39.8*/ 0.10036536023292207, /*-39.7*/ 0.10094677085000524,
        /*-39.6*/ 0.10153154954452942, /*-39.5*/ 0.10211971582756875, /*-39.4*/ 0.10271128932322379, /*-39.3*/ 0.10330628976927647, /*-39.2*/ 0.10390473701784844,
        /*-39.1*/ 0.10450665103606369, /*-39*/ 0.10511205190671431, /*-38.9*/ 0.10572095982893107, /*-38.8*/ 0.10633339511885699, /*-38.7*/ 0.10694937821032527,
        /*-38.6*/ 0.10756892965554125, /*-38.5*/ 0.10819207012576787, /*-38.4*/ 0.10881882041201554, /*-38.3*/ 0.10944920142573566, /*-38.2*/ 0.11008323419951854,
        /*-38.1*/ 0.11072093988779505, /*-38*/ 0.11136233976754242, /*-37.9*/ 0.11200745523899407, /*-37.8*/ 0.11265630782635379, /*-37.7*/ 0.11330891917851377,
        /*-37.6*/ 0.1139653110697771, /*-37.5*/ 0.1146255054005839, /*-37.4*/ 0.1152895241982424, /*-37.3*/ 0.11595738961766372, /*-37.2*/ 0.11662912394210093,
        /*-37.1*/ 0.11730474958389288, /*-37*/ 0.11798428908521168, /*-36.9*/ 0.11866776511881492, /*-36.8*/ 0.11935520048880209, /*-36.7*/ 0.12004661813137552,
        /*-36.6*/ 0.12074204111560567, /*-36.5*/ 0.12144149264420075, /*-36.4*/ 0.12214499605428075, /*-36.3*/ 0.1228525748181564, /*-36.2*/ 0.12356425254411198,
        /*-36.1*/ 0.12428005297719343, /*-36*/ 0.125, /*-35.9*/ 0.12572411763348168, /*-35.8*/ 0.12645243003774034, /*-35.7*/ 0.1271849615128358,
        /*-35.6*/ 0.12792173649959684, /*-35.5*/ 0.12866277958043648, /*-35.4*/ 0.12940811548017223, /*-35.3*/ 0.1301577690668508, /*-35.2*/ 0.1309117653525783,
        /*-35.1*/ 0.13167012949435447, /*-35*/ 0.1324328867949119, /*-34.9*/ 0.13320006270356055, /*-34.8*/ 0.13397168281703664, /*-34.7*/ 0.13474777288035686,
        /*-34.6*/ 0.1355283587876773, /*-34.5*/ 0.1363134665831572, /*-34.4*/ 0.13710312246182824, /*-34.3*/ 0.13789735277046836, /*-34.2*/ 0.1386961840084806,
        /*-34.1*/ 0.1394996428287779, /*-34*/ 0.1403077560386716, /*-33.9*/ 0.14112055060076642, /*-33.8*/ 0.1419380536338597, /*-33.7*/ 0.1427602924138464,
        /*-33.6*/ 0.14358729437462936, /*-33.5*/ 0.14441908710903414, /*-33.4*/ 0.14525569836972996, /*-33.3*/ 0.1460971560701557, /*-33.2*/ 0.14694348828545106,
        /*-33.1*/ 0.14779472325339352, /*-33*/ 0.14865088937534013, /*-32.9*/ 0.14951201521717547, /*-32.8*/ 0.15037812951026464, /*-32.7*/ 0.15124926115241158,
        /*-32.6*/ 0.15212543920882385, /*-32.5*/ 0.1530066929130819, /*-32.4*/ 0.15389305166811457, /*-32.3*/ 0.15478454504718006, /*-32.2*/ 0.15568120279485284,
        /*-32.1*/ 0.15658305482801596, /*-32*/ 0.15749013123685915, /*-31.9*/ 0.15840246228588278, /*-31.8*/ 0.15932007841490778, /*-31.7*/ 0.16024301024009113,
        /*-31.6*/ 0.1611712885549474, /*-31.5*/ 0.1621049443313762, /*-31.4*/ 0.16304400872069552, /*-31.3*/ 0.1639885130546811, /*-31.2*/ 0.16493848884661177,
        /*-31.1*/ 0.165893967792321, /*-31*/ 0.16685498177125427, /*-30.9*/ 0.16782156284753297, /*-30.8*/ 0.16879374327102373, /*-30.7*/ 0.1697715554784152,
        /*-30.6*/ 0.1707550320942994, /*-30.5*/ 0.17174420593226117, /*-30.4*/ 0.17273910999597203, /*-30.3*/ 0.17373977748029215, /*-30.2*/ 0.17474624177237744,
        /*-30.1*/ 0.17575853645279368, /*-30*/ 0.1767766952966369, /*-29.9*/ 0.1778007522746602, /*-29.8*/ 0.17883074155440729, /*-29.7*/ 0.17986669750135248,
        /*-29.6*/ 0.18090865468004724, /*-29.5*/ 0.1819566478552734, /*-29.4*/ 0.1830107119932032, /*-29.3*/ 0.18407088226256568, /*-29.2*/ 0.18513719403582055,
        /*-29.1*/ 0.18620968289033776, /*-29*/ 0.18728838460958522, /*-28.9*/ 0.188373335184322, /*-28.8*/ 0.18946457081379978, /*-28.7*/ 0.19056212790697025,
        /*-28.6*/ 0.19166604308370008, /*-28.5*/ 0.1927763531759926, /*-28.4*/ 0.19389309522921683, /*-28.3*/ 0.19501630650334337, /*-28.2*/ 0.19614602447418766,
        /*-28.1*/ 0.19728228683466037, /*-28*/ 0.19842513149602492, /*-27.9*/ 0.19957459658916252, /*-27.8*/ 0.2007307204658441, /*-27.7*/ 0.20189354170001053,
        /*-27.6*/ 0.20306309908905884, /*-27.5*/ 0.2042394316551375, /*-27.4*/ 0.20542257864644758, /*-27.3*/ 0.20661257953855294, /*-27.2*/ 0.20780947403569694,
        /*-27.1*/ 0.20901330207212737, /*-27*/ 0.21022410381342863, /*-26.9*/ 0.21144191965786213, /*-26.8*/ 0.21266679023771393, /*-26.7*/ 0.21389875642065054,
        /*-26.6*/ 0.2151378593110825, /*-26.5*/ 0.21638414025153574, /*-26.4*/ 0.2176376408240311, /*-26.3*/ 0.21889840285147127, /*-26.2*/ 0.22016646839903714,
        /*-26.1*/ 0.2214418797755901, /*-26*/ 0.22272467953508485, /*-25.9*/ 0.22401491047798813, /*-25.8*/ 0.22531261565270758, /*-25.7*/ 0.22661783835702762,
        /*-25.6*/ 0.2279306221395542, /*-25.5*/ 0.2292510108011678, /*-25.4*/ 0.2305790483964848, /*-25.3*/ 0.23191477923532736, /*-25.2*/ 0.23325824788420185,
        /*-25.1*/ 0.23460949916778576, /*-25*/ 0.23596857817042335, /*-24.9*/ 0.23733553023762985, /*-24.8*/ 0.2387104009776041, /*-24.7*/ 0.2400932362627511,
        /*-24.6*/ 0.24148408223121134, /*-24.5*/ 0.2428829852884015, /*-24.4*/ 0.2442899921085615, /*-24.3*/ 0.2457051496363128, /*-24.2*/ 0.24712850508822404,
        /*-24.1*/ 0.24856010595438685, /*-24*/ 0.25, /*-23.9*/ 0.2514482352669634, /*-23.8*/ 0.25290486007548063, /*-23.7*/ 0.25436992302567163,
        /*-23.6*/ 0.2558434729991937, /*-23.5*/ 0.257325559160873, /*-23.4*/ 0.2588162309603444, /*-23.3*/ 0.2603155381337016, /*-23.2*/ 0.2618235307051567,
        /*-23.1*/ 0.26334025898870894, /*-23*/ 0.2648657735898238, /*-22.9*/ 0.2664001254071211, /*-22.8*/ 0.2679433656340733, /*-22.7*/ 0.2694955457607138,
        /*-22.6*/ 0.27105671757535454, /*-22.5*/ 0.2726269331663144, /*-22.4*/ 0.27420624492365653, /*-22.3*/ 0.2757947055409366, /*-22.2*/ 0.27739236801696127,
        /*-22.1*/ 0.2789992856575558, /*-22*/ 0.28061551207734325, /*-21.9*/ 0.28224110120153284, /*-21.8*/ 0.28387610726771934, /*-21.7*/ 0.2855205848276929,
        /*-21.6*/ 0.2871745887492587, /*-21.5*/ 0.28883817421806823, /*-21.4*/ 0.2905113967394599, /*-21.3*/ 0.29219431214031133, /*-21.2*/ 0.2938869765709022,
        /*-21.1*/ 0.295589446506787, /*-21*/ 0.29730177875068026, /*-20.9*/ 0.299024030434351, /*-20.8*/ 0.30075625902052916, /*-20.7*/ 0.30249852230482316,
        /*-20.6*/ 0.3042508784176477, /*-20.5*/ 0.30601338582616383, /*-20.4*/ 0.3077861033362291, /*-20.3*/ 0.3095690900943601, /*-20.2*/ 0.31136240558970574,
        /*-20.1*/ 0.313166109656032, /*-20*/ 0.3149802624737183, /*-19.9*/ 0.31680492457176557, /*-19.8*/ 0.3186401568298155, /*-19.7*/ 0.32048602048018227,
        /*-19.6*/ 0.3223425771098948, /*-19.5*/ 0.3242098886627524, /*-19.4*/ 0.3260880174413911, /*-19.3*/ 0.3279770261093622, /*-19.2*/ 0.3298769776932236,
        /*-19.1*/ 0.331787935584642, /*-19*/ 0.3337099635425086, /*-18.9*/ 0.3356431256950659, /*-18.8*/ 0.3375874865420475, /*-18.7*/ 0.3395431109568303,
        /*-18.6*/ 0.34151006418859886, /*-18.5*/ 0.3434884118645223, /*-18.4*/ 0.34547821999194406, /*-18.3*/ 0.34747955496058425, /*-18.2*/ 0.3494924835447549,
        /*-18.1*/ 0.3515170729055873, /*-18*/ 0.3535533905932738, /*-17.9*/ 0.35560150454932044, /*-17.8*/ 0.35766148310881457, /*-17.7*/ 0.359733395002705,
        /*-17.6*/ 0.3618173093600945, /*-17.5*/ 0.36391329571054687, /*-17.4*/ 0.36602142398640636, /*-17.3*/ 0.3681417645251314, /*-17.2*/ 0.37027438807164104,
        /*-17.1*/ 0.3724193657806756, /*-17*/ 0.3745767692191704, /*-16.9*/ 0.376746670368644, /*-16.8*/ 0.3789291416275995, /*-16.7*/ 0.3811242558139405,
        /*-16.6*/ 0.3833320861674001, /*-16.5*/ 0.3855527063519852, /*-16.4*/ 0.3877861904584337, /*-16.3*/ 0.39003261300668673, /*-16.2*/ 0.3922920489483754,
        /*-16.1*/ 0.39456457366932074, /*-16*/ 0.3968502629920499, /*-15.9*/ 0.399149193178325, /*-15.8*/ 0.40146144093168823, /*-15.7*/ 0.403787083400021,
        /*-15.6*/ 0.40612619817811774, /*-15.5*/ 0.40847886331027494, /*-15.4*/ 0.4108451572928951, /*-15.3*/ 0.41322515907710583, /*-15.2*/ 0.4156189480713939,
        /*-15.1*/ 0.41802660414425474, /*-15*/ 0.42044820762685725, /*-14.9*/ 0.42288383931572426, /*-14.8*/ 0.42533358047542785, /*-14.7*/ 0.42779751284130113,
        /*-14.6*/ 0.43027571862216507, /*-14.5*/ 0.43276828050307153, /*-14.4*/ 0.43527528164806206, /*-14.3*/ 0.4377968057029426, /*-14.2*/ 0.4403329367980742,
        /*-14.1*/ 0.44288375955118026, /*-14*/ 0.44544935907016964, /*-13.9*/ 0.4480298209559762, /*-13.8*/ 0.4506252313054151, /*-13.7*/ 0.45323567671405524,
        /*-13.6*/ 0.4558612442791084, /*-13.5*/ 0.4585020216023356, /*-13.4*/ 0.4611580967929696, /*-13.3*/ 0.4638295584706547, /*-13.2*/ 0.46651649576840376,
        /*-13.1*/ 0.4692189983355716, /*-13*/ 0.47193715634084676, /*-12.9*/ 0.47467106047525964, /*-12.8*/ 0.4774208019552083, /*-12.7*/ 0.4801864725255021,
        /*-12.6*/ 0.48296816446242274, /*-12.5*/ 0.48576597057680293, /*-12.4*/ 0.48857998421712295, /*-12.3*/ 0.4914102992726255, /*-12.2*/ 0.4942570101764481,
        /*-12.1*/ 0.4971202119087737, /*-12*/ 0.5, /*-11.9*/ 0.5028964705339267, /*-11.8*/ 0.5058097201509613, /*-11.7*/ 0.5087398460513433,
        /*-11.6*/ 0.5116869459983875, /*-11.5*/ 0.514651118321746, /*-11.4*/ 0.5176324619206887, /*-11.3*/ 0.5206310762674031, /*-11.2*/ 0.5236470614103134,
        /*-11.1*/ 0.526680517977418, /*-11*/ 0.5297315471796477, /*-10.9*/ 0.5328002508142422, /*-10.8*/ 0.5358867312681466, /*-10.7*/ 0.5389910915214275,
        /*-10.6*/ 0.5421134351507092, /*-10.5*/ 0.5452538663326288, /*-10.4*/ 0.548412489847313, /*-10.3*/ 0.5515894110818732, /*-10.2*/ 0.5547847360339225,
        /*-10.1*/ 0.5579985713151117, /*-10*/ 0.5612310241546865, /*-9.9*/ 0.5644822024030656, /*-9.8*/ 0.5677522145354387, /*-9.7*/ 0.5710411696553858,
        /*-9.6*/ 0.5743491774985175, /*-9.5*/ 0.5776763484361365, /*-9.4*/ 0.5810227934789198, /*-9.3*/ 0.5843886242806228, /*-9.2*/ 0.5877739531418044,
        /*-9.1*/ 0.5911788930135741, /*-9*/ 0.5946035575013605, /*-8.9*/ 0.5980480608687019, /*-8.8*/ 0.6015125180410583, /*-8.7*/ 0.6049970446096463,
        /*-8.6*/ 0.6085017568352955, /*-8.5*/ 0.6120267716523276, /*-8.4*/ 0.6155722066724582, /*-8.3*/ 0.6191381801887201, /*-8.2*/ 0.6227248111794115,
        /*-8.1*/ 0.626332219312064, /*-8*/ 0.6299605249474366, /*-7.9*/ 0.6336098491435311, /*-7.8*/ 0.6372803136596311, /*-7.7*/ 0.6409720409603644,
        /*-7.6*/ 0.6446851542197896, /*-7.5*/ 0.6484197773255048, /*-7.4*/ 0.6521760348827821, /*-7.3*/ 0.6559540522187244, /*-7.2*/ 0.6597539553864471,
        /*-7.1*/ 0.6635758711692841, /*-7*/ 0.6674199270850172, /*-6.9*/ 0.6712862513901316, /*-6.8*/ 0.675174973084095, /*-6.7*/ 0.6790862219136606,
        /*-6.6*/ 0.6830201283771978, /*-6.5*/ 0.6869768237290446, /*-6.4*/ 0.690956439983888, /*-6.3*/ 0.6949591099211685, /*-6.2*/ 0.6989849670895096,
        /*-6.1*/ 0.7030341458111747, /*-6*/ 0.7071067811865476, /*-5.9*/ 0.7112030090986408, /*-5.8*/ 0.7153229662176291, /*-5.7*/ 0.71946679000541,
        /*-5.6*/ 0.7236346187201891, /*-5.5*/ 0.7278265914210937, /*-5.4*/ 0.7320428479728127, /*-5.3*/ 0.7362835290502628, /*-5.2*/ 0.7405487761432821,
        /*-5.1*/ 0.7448387315613512, /*-5*/ 0.7491535384383408, /*-4.9*/ 0.7534933407372879, /*-4.8*/ 0.7578582832551991, /*-4.7*/ 0.762248511627881,
        /*-4.6*/ 0.7666641723348003, /*-4.5*/ 0.7711054127039704, /*-4.4*/ 0.7755723809168673, /*-4.3*/ 0.7800652260133735, /*-4.2*/ 0.7845840978967508,
        /*-4.1*/ 0.7891291473386416, /*-4*/ 0.7937005259840998, /*-3.9*/ 0.7982983863566498, /*-3.8*/ 0.8029228818633765, /*-3.7*/ 0.807574166800042,
        /*-3.6*/ 0.8122523963562356, /*-3.5*/ 0.8169577266205499, /*-3.4*/ 0.8216903145857903, /*-3.3*/ 0.8264503181542118, /*-3.2*/ 0.8312378961427878,
        /*-3.1*/ 0.8360532082885094, /*-3*/ 0.8408964152537145, /*-2.9*/ 0.8457676786314485, /*-2.8*/ 0.8506671609508557, /*-2.7*/ 0.8555950256826022,
        /*-2.6*/ 0.86055143724433, /*-2.5*/ 0.8655365610061431, /*-2.4*/ 0.8705505632961241, /*-2.3*/ 0.8755936114058852, /*-2.2*/ 0.8806658735961485,
        /*-2.1*/ 0.8857675191023606, /*-2*/ 0.8908987181403393, /*-1.9*/ 0.8960596419119524, /*-1.8*/ 0.9012504626108302, /*-1.7*/ 0.9064713534281105,
        /*-1.6*/ 0.9117224885582168, /*-1.5*/ 0.9170040432046712, /*-1.4*/ 0.9223161935859392, /*-1.3*/ 0.9276591169413094, /*-1.2*/ 0.9330329915368074,
        /*-1.1*/ 0.9384379966711431, /*-1*/ 0.9438743126816935, /*-0.9*/ 0.9493421209505192, /*-0.8*/ 0.9548416039104165, /*-0.7*/ 0.9603729450510042,
        /*-0.6*/ 0.9659363289248456, /*-0.5*/ 0.9715319411536059, /*-0.4*/ 0.9771599684342459, /*-0.3*/ 0.9828205985452511, /*-0.2*/ 0.9885140203528962,
        /*-0.1*/ 0.9942404238175473, /*0*/ 1, /*0.1*/ 1.0057929410678534, /*0.2*/ 1.0116194403019225, /*0.3*/ 1.0174796921026863,
        /*0.4*/ 1.023373891996775, /*0.5*/ 1.029302236643492, /*0.6*/ 1.0352649238413776, /*0.7*/ 1.0412621525348065, /*0.8*/ 1.0472941228206267,
        /*0.9*/ 1.0533610359548358, /*1*/ 1.0594630943592953, /*1.1*/ 1.0656005016284844, /*1.2*/ 1.0717734625362931, /*1.3*/ 1.077982183042855,
        /*1.4*/ 1.0842268703014184, /*1.5*/ 1.0905077326652577, /*1.6*/ 1.096824979694626, /*1.7*/ 1.1031788221637464, /*1.8*/ 1.109569472067845,
        /*1.9*/ 1.1159971426302233, /*2*/ 1.122462048309373, /*2.1*/ 1.1289644048061311, /*2.2*/ 1.1355044290708773, /*2.3*/ 1.1420823393107715,
        /*2.4*/ 1.148698354997035, /*2.5*/ 1.155352696872273, /*2.6*/ 1.1620455869578397, /*2.7*/ 1.1687772485612455, /*2.8*/ 1.1755479062836087,
        /*2.9*/ 1.1823577860271481, /*3*/ 1.189207115002721, /*3.1*/ 1.1960961217374038, /*3.2*/ 1.2030250360821166, /*3.3*/ 1.2099940892192926,
        /*3.4*/ 1.217003513670591, /*3.5*/ 1.2240535433046553, /*3.6*/ 1.2311444133449163, /*3.7*/ 1.2382763603774405, /*3.8*/ 1.245449622358823,
        /*3.9*/ 1.252664438624128, /*4*/ 1.2599210498948732, /*4.1*/ 1.267219698287062, /*4.2*/ 1.2745606273192622, /*4.3*/ 1.2819440819207288,
        /*4.4*/ 1.2893703084395791, /*4.5*/ 1.2968395546510096, /*4.6*/ 1.3043520697655642, /*4.7*/ 1.3119081044374488, /*4.8*/ 1.3195079107728942,
        /*4.9*/ 1.3271517423385681, /*5*/ 1.3348398541700344, /*5.1*/ 1.3425725027802635, /*5.2*/ 1.35034994616819, /*5.3*/ 1.3581724438273213,
        /*5.4*/ 1.3660402567543954, /*5.5*/ 1.3739536474580891, /*5.6*/ 1.381912879967776, /*5.7*/ 1.389918219842337, /*5.8*/ 1.3979699341790195,
        /*5.9*/ 1.4060682916223495, /*6*/ 1.4142135623730951, /*6.1*/ 1.4224060181972815, /*6.2*/ 1.4306459324352585, /*6.3*/ 1.43893358001082,
        /*6.4*/ 1.4472692374403782, /*6.5*/ 1.4556531828421873, /*6.6*/ 1.4640856959456254, /*6.7*/ 1.4725670581005257, /*6.8*/ 1.4810975522865641,
        /*6.9*/ 1.4896774631227023, /*7*/ 1.4983070768766815, /*7.1*/ 1.5069866814745758, /*7.2*/ 1.515716566510398, /*7.3*/ 1.524497023255762,
        /*7.4*/ 1.5333283446696007, /*7.5*/ 1.5422108254079407, /*7.6*/ 1.5511447618337346, /*7.7*/ 1.560130452026747, /*7.8*/ 1.5691681957935015,
        /*7.9*/ 1.5782582946772832, /*8*/ 1.5874010519681994, /*8.1*/ 1.5965967727132997, /*8.2*/ 1.605845763726753, /*8.3*/ 1.6151483336000843,
        /*8.4*/ 1.6245047927124712, /*8.5*/ 1.6339154532411, /*8.6*/ 1.6433806291715807, /*8.7*/ 1.6529006363084233, /*8.8*/ 1.6624757922855755,
        /*8.9*/ 1.672106416577019, /*9*/ 1.681792830507429, /*9.1*/ 1.691535357262897, /*9.2*/ 1.7013343219017114, /*9.3*/ 1.7111900513652045,
        /*9.4*/ 1.72110287448866, /*9.5*/ 1.731073122012286, /*9.6*/ 1.7411011265922482, /*9.7*/ 1.7511872228117702, /*9.8*/ 1.761331747192297,
        /*9.9*/ 1.7715350382047212, /*10*/ 1.7817974362806785, /*10.1*/ 1.7921192838239048, /*10.2*/ 1.8025009252216604, /*10.3*/ 1.812942706856221,
        /*10.4*/ 1.8234449771164336, /*10.5*/ 1.8340080864093424, /*10.6*/ 1.8446323871718784, /*10.7*/ 1.8553182338826189, /*10.8*/ 1.8660659830736148,
        /*10.9*/ 1.8768759933422863, /*11*/ 1.8877486253633868, /*11.1*/ 1.8986842419010383, /*11.2*/ 1.909683207820833, /*11.3*/ 1.9207458901020087,
        /*11.4*/ 1.9318726578496912, /*11.5*/ 1.9430638823072117, /*11.6*/ 1.9543199368684918, /*11.7*/ 1.9656411970905021, /*11.8*/ 1.9770280407057923,
        /*11.9*/ 1.9884808476350948, /*12*/ 2, /*12.1*/ 2.011585882135707, /*12.2*/ 2.023238880603845, /*12.3*/ 2.034959384205373,
        /*12.4*/ 2.04674778399355, /*12.5*/ 2.058604473286984, /*12.6*/ 2.070529847682755, /*12.7*/ 2.082524305069613, /*12.8*/ 2.0945882456412535,
        /*12.9*/ 2.1067220719096715, /*13*/ 2.1189261887185906, /*13.1*/ 2.1312010032569684, /*13.2*/ 2.1435469250725863, /*13.3*/ 2.15596436608571,
        /*13.4*/ 2.168453740602837, /*13.5*/ 2.1810154653305154, /*13.6*/ 2.193649959389252, /*13.7*/ 2.206357644327493, /*13.8*/ 2.21913894413569,
        /*13.9*/ 2.2319942852604466, /*14*/ 2.244924096618746, /*14.1*/ 2.2579288096122627, /*14.2*/ 2.2710088581417547, /*14.3*/ 2.284164678621543,
        /*14.4*/ 2.2973967099940698, /*14.5*/ 2.310705393744546, /*14.6*/ 2.324091173915679, /*14.7*/ 2.3375544971224906, /*14.8*/ 2.3510958125672174,
        /*14.9*/ 2.3647155720542963, /*15*/ 2.378414230005442, /*15.1*/ 2.3921922434748075, /*15.2*/ 2.4060500721642333, /*15.3*/ 2.4199881784385853,
        /*15.4*/ 2.434007027341182, /*15.5*/ 2.4481070866093106, /*15.6*/ 2.4622888266898326, /*15.7*/ 2.476552720754881, /*15.8*/ 2.490899244717646,
        /*15.9*/ 2.505328877248256, /*16*/ 2.5198420997897464, /*16.1*/ 2.5344393965741245, /*16.2*/ 2.549121254638524, /*16.3*/ 2.563888163841458,
        /*16.4*/ 2.5787406168791582, /*16.5*/ 2.5936791093020193, /*16.6*/ 2.608704139531129, /*16.7*/ 2.6238162088748975, /*16.8*/ 2.639015821545789,
        /*16.9*/ 2.654303484677136, /*17*/ 2.6696797083400687, /*17.1*/ 2.685145005560527, /*17.2*/ 2.70069989233638, /*17.3*/ 2.7163448876546425,
        /*17.4*/ 2.732080513508791, /*17.5*/ 2.7479072949161782, /*17.6*/ 2.7638257599355525, /*17.7*/ 2.779836439684674, /*17.8*/ 2.795939868358039,
        /*17.9*/ 2.8121365832446985, /*18*/ 2.8284271247461903, /*18.1*/ 2.8448120363945635, /*18.2*/ 2.8612918648705166, /*18.3*/ 2.87786716002164,
        /*18.4*/ 2.894538474880756, /*18.5*/ 2.911306365684375, /*18.6*/ 2.928171391891251, /*18.7*/ 2.9451341162010514, /*18.8*/ 2.9621951045731283,
        /*18.9*/ 2.9793549262454047, /*19*/ 2.996614153753363, /*19.1*/ 3.013973362949152, /*19.2*/ 3.031433133020796, /*19.3*/ 3.048994046511524,
        /*19.4*/ 3.066656689339201, /*19.5*/ 3.0844216508158815, /*19.6*/ 3.1022895236674697, /*19.7*/ 3.120260904053494, /*19.8*/ 3.138336391587003,
        /*19.9*/ 3.156516589354566, /*20*/ 3.174802103936399, /*20.1*/ 3.1931935454266, /*20.2*/ 3.211691527453506, /*20.3*/ 3.230296667200168,
        /*20.4*/ 3.249009585424942, /*20.5*/ 3.2678309064821995, /*20.6*/ 3.2867612583431614, /*20.7*/ 3.3058012726168466, /*20.8*/ 3.324951584571151,
        /*20.9*/ 3.344212833154037, /*21*/ 3.363585661014858, /*21.1*/ 3.3830707145257946, /*21.2*/ 3.402668643803423, /*21.3*/ 3.422380102730409,
        /*21.4*/ 3.44220574897732, /*21.5*/ 3.4621462440245723, /*21.6*/ 3.4822022531844965, /*21.7*/ 3.502374445623541, /*21.8*/ 3.522663494384594,
        /*21.9*/ 3.543070076409442, /*22*/ 3.563594872561357, /*22.1*/ 3.58423856764781, /*22.2*/ 3.6050018504433208, /*22.3*/ 3.625885413712442,
        /*22.4*/ 3.6468899542328668, /*22.5*/ 3.668016172818685, /*22.6*/ 3.689264774343757, /*22.7*/ 3.7106364677652377, /*22.8*/ 3.73213196614723,
        /*22.9*/ 3.753751986684572, /*23*/ 3.775497250726774, /*23.1*/ 3.797368483802077, /*23.2*/ 3.8193664156416665, /*23.3*/ 3.841491780204017,
        /*23.4*/ 3.863745315699382, /*23.5*/ 3.8861277646144234, /*23.6*/ 3.908639873736984, /*23.7*/ 3.931282394181004, /*23.8*/ 3.9540560814115846,
        /*23.9*/ 3.9769616952701887, /*24*/ 4, /*24.1*/ 4.023171764271414, /*24.2*/ 4.04647776120769, /*24.3*/ 4.069918768410745,
        /*24.4*/ 4.093495567987099, /*24.5*/ 4.117208946573967, /*24.6*/ 4.141059695365511, /*24.7*/ 4.165048610139225, /*24.8*/ 4.189176491282508,
        /*24.9*/ 4.213444143819343, /*25*/ 4.237852377437181, /*25.1*/ 4.262402006513938, /*25.2*/ 4.2870938501451725, /*25.3*/ 4.31192873217142,
        /*25.4*/ 4.336907481205674, /*25.5*/ 4.362030930661031, /*25.6*/ 4.387299918778504, /*25.7*/ 4.412715288654986, /*25.8*/ 4.438277888271379,
        /*25.9*/ 4.463988570520892, /*26*/ 4.489848193237491, /*26.1*/ 4.515857619224525, /*26.2*/ 4.5420177162835085, /*26.3*/ 4.568329357243087,
        /*26.4*/ 4.5947934199881395, /*26.5*/ 4.621410787489093, /*26.6*/ 4.648182347831359, /*26.7*/ 4.675108994244982, /*26.8*/ 4.702191625134435,
        /*26.9*/ 4.7294311441085926, /*27*/ 4.756828460010884, /*27.1*/ 4.784384486949615, /*27.2*/ 4.8121001443284666, /*27.3*/ 4.8399763568771705,
        /*27.4*/ 4.868014054682363, /*27.5*/ 4.89621417321862, /*27.6*/ 4.924577653379666, /*27.7*/ 4.953105441509761, /*27.8*/ 4.981798489435293,
        /*27.9*/ 5.010657754496511, /*28*/ 5.039684199579493, /*28.1*/ 5.068878793148249, /*28.2*/ 5.098242509277049, /*28.3*/ 5.127776327682916,
        /*28.4*/ 5.1574812337583165, /*28.5*/ 5.187358218604039, /*28.6*/ 5.217408279062257, /*28.7*/ 5.247632417749795, /*28.8*/ 5.278031643091577,
        /*28.9*/ 5.308606969354272, /*29*/ 5.339359416680137, /*29.1*/ 5.370290011121055, /*29.2*/ 5.401399784672759, /*29.3*/ 5.432689775309286,
        /*29.4*/ 5.464161027017581, /*29.5*/ 5.495814589832357, /*29.6*/ 5.527651519871105, /*29.7*/ 5.559672879369349, /*29.8*/ 5.591879736716078,
        /*29.9*/ 5.624273166489398, /*30*/ 5.656854249492381, /*30.1*/ 5.689624072789126, /*30.2*/ 5.722583729741033, /*30.3*/ 5.755734320043279,
        /*30.4*/ 5.789076949761512, /*30.5*/ 5.822612731368749, /*30.6*/ 5.856342783782503, /*30.7*/ 5.890268232402102, /*30.8*/ 5.9243902091462575,
        /*30.9*/ 5.958709852490808, /*31*/ 5.993228307506727, /*31.1*/ 6.027946725898304, /*31.2*/ 6.062866266041593, /*31.3*/ 6.097988093023048,
        /*31.4*/ 6.133313378678403, /*31.5*/ 6.168843301631763, /*31.6*/ 6.2045790473349385, /*31.7*/ 6.240521808106988, /*31.8*/ 6.276672783174005,
        /*31.9*/ 6.313033178709132, /*32*/ 6.3496042078727974, /*32.1*/ 6.3863870908532006, /*32.2*/ 6.423383054907013, /*32.3*/ 6.460593334400335,
        /*32.4*/ 6.498019170849883, /*32.5*/ 6.5356618129644, /*32.6*/ 6.573522516686323, /*32.7*/ 6.611602545233694, /*32.8*/ 6.6499031691423,
        /*32.9*/ 6.688425666308076, /*33*/ 6.727171322029716, /*33.1*/ 6.766141429051588, /*33.2*/ 6.805337287606847, /*33.3*/ 6.844760205460817,
        /*33.4*/ 6.88441149795464, /*33.5*/ 6.924292488049144, /*33.6*/ 6.964404506368995, /*33.7*/ 7.0047488912470826, /*33.8*/ 7.045326988769187,
        /*33.9*/ 7.086140152818883, /*34*/ 7.127189745122715, /*34.1*/ 7.16847713529562, /*34.2*/ 7.210003700886642, /*34.3*/ 7.251770827424881,
        /*34.4*/ 7.293779908465734, /*34.5*/ 7.33603234563737, /*34.6*/ 7.3785295486875135, /*34.7*/ 7.421272935530478, /*34.8*/ 7.464263932294459,
        /*34.9*/ 7.507503973369144, /*35*/ 7.550994501453547, /*35.1*/ 7.594736967604155, /*35.2*/ 7.638732831283334, /*35.3*/ 7.682983560408033,
        /*35.4*/ 7.727490631398763, /*35.5*/ 7.772255529228848, /*35.6*/ 7.817279747473968, /*35.7*/ 7.862564788362009, /*35.8*/ 7.9081121628231665,
        /*35.9*/ 7.953923390540379, /*36*/ 8, /*36.1*/ 8.046343528542828, /*36.2*/ 8.092955522415382, /*36.3*/ 8.13983753682149,
        /*36.4*/ 8.186991135974198, /*36.5*/ 8.234417893147935, /*36.6*/ 8.282119390731022, /*36.7*/ 8.330097220278452, /*36.8*/ 8.378352982565012,
        /*36.9*/ 8.426888287638686, /*37*/ 8.475704754874362, /*37.1*/ 8.524804013027875, /*37.2*/ 8.574187700290345, /*37.3*/ 8.623857464342839,
        /*37.4*/ 8.673814962411347, /*37.5*/ 8.724061861322062, /*37.6*/ 8.774599837557007, /*37.7*/ 8.825430577309975, /*37.8*/ 8.876555776542759,
        /*37.9*/ 8.927977141041785, /*38*/ 8.979696386474982, /*38.1*/ 9.03171523844905, /*38.2*/ 9.08403543256702, /*38.3*/ 9.13665871448617,
        /*38.4*/ 9.189586839976279, /*38.5*/ 9.242821574978185, /*38.6*/ 9.296364695662717, /*38.7*/ 9.350217988489964, /*38.8*/ 9.404383250268868,
        /*38.9*/ 9.458862288217185, /*39*/ 9.513656920021768, /*39.1*/ 9.56876897389923, /*39.2*/ 9.624200288656937, /*39.3*/ 9.679952713754341,
        /*39.4*/ 9.736028109364726, /*39.5*/ 9.79242834643724, /*39.6*/ 9.849155306759332, /*39.7*/ 9.906210883019524, /*39.8*/ 9.963596978870582,
        /*39.9*/ 10.021315508993021, /*40*/ 10.079368399158986, /*40.1*/ 10.137757586296498, /*40.2*/ 10.196485018554098, /*40.3*/ 10.255552655365829,
        /*40.4*/ 10.314962467516633, /*40.5*/ 10.374716437208077, /*40.6*/ 10.434816558124513, /*40.7*/ 10.495264835499594, /*40.8*/ 10.556063286183154,
        /*40.9*/ 10.617213938708543, /*41*/ 10.678718833360273, /*41.1*/ 10.74058002224211, /*41.2*/ 10.802799569345522, /*41.3*/ 10.865379550618568,
        /*41.4*/ 10.928322054035162, /*41.5*/ 10.991629179664715, /*41.6*/ 11.05530303974221, /*41.7*/ 11.119345758738698, /*41.8*/ 11.183759473432152,
        /*41.9*/ 11.248546332978796, /*42*/ 11.313708498984761, /*42.1*/ 11.379248145578252, /*42.2*/ 11.44516745948207, /*42.3*/ 11.511468640086559,
        /*42.4*/ 11.578153899523024, /*42.5*/ 11.645225462737498, /*42.6*/ 11.712685567565005, /*42.7*/ 11.780536464804207, /*42.8*/ 11.848780418292511,
        /*42.9*/ 11.917419704981617, /*43*/ 11.986456615013454, /*43.1*/ 12.055893451796608, /*43.2*/ 12.125732532083186, /*43.3*/ 12.195976186046092,
        /*43.4*/ 12.266626757356805, /*43.5*/ 12.337686603263526, /*43.6*/ 12.409158094669877, /*43.7*/ 12.481043616213979, /*43.8*/ 12.55334556634801,
        /*43.9*/ 12.626066357418264, /*44*/ 12.699208415745595, /*44.1*/ 12.772774181706401, /*44.2*/ 12.846766109814025, /*44.3*/ 12.92118666880067,
        /*44.4*/ 12.996038341699766, /*44.5*/ 13.0713236259288, /*44.6*/ 13.147045033372645, /*44.7*/ 13.223205090467388, /*44.8*/ 13.2998063382846,
        /*44.9*/ 13.376851332616152, /*45*/ 13.454342644059432, /*45.1*/ 13.532282858103176, /*45.2*/ 13.610674575213695, /*45.3*/ 13.689520410921634,
        /*45.4*/ 13.76882299590928, /*45.5*/ 13.848584976098287, /*45.6*/ 13.92880901273799, /*45.7*/ 14.009497782494165, /*45.8*/ 14.090653977538373,
        /*45.9*/ 14.172280305637766, /*46*/ 14.25437949024543, /*46.1*/ 14.33695427059124, /*46.2*/ 14.420007401773285, /*46.3*/ 14.503541654849762,
        /*46.4*/ 14.587559816931469, /*46.5*/ 14.67206469127474, /*46.6*/ 14.757059097375027, /*46.7*/ 14.842545871060956, /*46.8*/ 14.928527864588919,
        /*46.9*/ 15.015007946738288, /*47*/ 15.101989002907095, /*47.1*/ 15.18947393520831, /*47.2*/ 15.277465662566668, /*47.3*/ 15.365967120816066,
        /*47.4*/ 15.454981262797526, /*47.5*/ 15.544511058457696, /*47.6*/ 15.634559494947936, /*47.7*/ 15.725129576724019, /*47.8*/ 15.816224325646333,
        /*47.9*/ 15.907846781080758, /*48*/ 16, /*48.1*/ 16.09268705708566, /*48.2*/ 16.18591104483076, /*48.3*/ 16.279675073642977,
        /*48.4*/ 16.373982271948396, /*48.5*/ 16.468835786295877, /*48.6*/ 16.564238781462038, /*48.7*/ 16.660194440556904, /*48.8*/ 16.756705965130024,
        /*48.9*/ 16.853776575277376, /*49*/ 16.95140950974872, /*49.1*/ 17.04960802605575, /*49.2*/ 17.148375400580697, /*49.3*/ 17.24771492868568,
        /*49.4*/ 17.347629924822687, /*49.5*/ 17.448123722644123, /*49.6*/ 17.54919967511402, /*49.7*/ 17.650861154619943, /*49.8*/ 17.753111553085514,
        /*49.9*/ 17.85595428208357, /*50*/ 17.959392772949972, /*50.1*/ 18.063430476898098, /*50.2*/ 18.16807086513404, /*50.3*/ 18.27331742897234,
        /*50.4*/ 18.37917367995256, /*50.5*/ 18.485643149956363, /*50.6*/ 18.592729391325435, /*50.7*/ 18.700435976979936, /*50.8*/ 18.80876650053774,
        /*50.9*/ 18.917724576434363, /*51*/ 19.027313840043536, /*51.1*/ 19.137537947798467 };
